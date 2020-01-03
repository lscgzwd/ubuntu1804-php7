#include "image.h"
#include "utils.h"
#include <cmath>
#include <limits.h>
#include <json/json.h>
#include <string.h>
#include <sstream>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <set>
#define BLACK_THRESHOLD (0xb0)
#define SHORT_LINE_LEN_MIN (20)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x) __builtin_expect(!!(x), 1)

//#define USE_DEPRECATED_POTENTIAL_BIT_MAP NICE

using namespace Magick;
using namespace std;

#define IS_BLACK(get_color, i, j) ((get_color((i), (j)) & 0xff) < BLACK_THRESHOLD)


std::string json_encode(const Json::Value &jv)
{
    Json::FastWriter fastWriter;
    std::stringstream outJson;
    outJson << fastWriter.write(jv);
    std::string s;
    s = outJson.str();
    return s;
}

bool json_decode(const std::string & json, Json::Value &root)
{
    std::stringstream ss;
    ss << json;
    Json::Reader reader;
    return reader.parse(ss, root, false);
}

std::vector<std::string>
json2vector(std::string str)
{
    std::vector<std::string> ret;
    Json::Value root;
    json_decode(str, root);
    if (root.type() == Json::ValueType::arrayValue) {
        for (int i = 0; i < root.size(); i++) {
            auto item = root[i];
            if (item.type() == Json::ValueType::stringValue) {
                ret.push_back(item.asString());
            }
        }
    }
    return ret;
}

template <typename T, typename K, typename V>
std::map<K, V> vector2map(
        std::vector<T> &v,
        std::function<K(T)> &getKey,
        std::function<V(T)> &getValue)
{
    std::map<K, V> ret;
    for_each(v.begin(), v.end(), [&ret, &getKey, &getValue](T item){
            ret[getKey(item)] = getValue(item);
    });
    return ret;
}

extern string tmp_image_file(void)
{
    char template_name[] = "/tmp/deskew-resize-XXXXXX";
    auto str = string(mktemp(template_name));
    str.append(".jpg");
    return str;
}

static inline bool is_black(
        function<unsigned int (unsigned int, unsigned int)> &get_color,
        unsigned int i,
        unsigned int j)
{
    unsigned int v00 = get_color(i - 3, j) & 0xff;
    unsigned int v01 = get_color(i - 2, j) & 0xff;
    unsigned int v02 = get_color(i - 1, j) & 0xff;
    unsigned int v0 = (v00 + v01 + v02) / 3;
    unsigned int v10 = get_color(i + 1, j) & 0xff;
    unsigned int v11 = get_color(i + 2, j) & 0xff;
    unsigned int v12 = get_color(i + 3, j) & 0xff;
    unsigned int v1 = (v10 + v11 + v12) / 3;
    unsigned int v = get_color(i, j) & 0xff;
    unsigned int t = (v + 1);
    return v < BLACK_THRESHOLD && (v0 > t || v1 > t);
}

simple_line simple_line::clone(const simple_line &line)
{
    simple_line ret;
    ret.first_dim = line.first_dim;
    ret.second_dim = line.second_dim;
    ret.len = line.len;
    return ret;
}

/**
  * 把粗线条变成细线条
  */
vector<simple_line>* sline_thin(vector<simple_line> &lines)
{
    map<int, set<int>> assoc_w2h;
    for (int i = 0; i < lines.size(); i++) {
        auto start = lines[i].second_dim;
        auto len = lines[i].len;
        for (int j = 0; j < len; j++) {
            auto index = j + start;
            assoc_w2h[index].insert(lines[i].first_dim);
        }
    }
    map<int, set<int>> assoc_h2w;
    for (auto itt = assoc_w2h.begin(); itt != assoc_w2h.end(); itt ++) {
        auto x = itt->first;
        auto foreach = [&itt](function<void(int)> cb){
            for (auto jtt = itt->second.begin(); jtt != itt->second.end(); jtt++) {
                cb(*jtt);
            }
        };
        auto islands = ContinuousNumber::islands(foreach);
        for (int i = 0; i < islands.size(); i++) {
            auto center = (islands[i][0] + (islands[i][0] + islands[i].size() - 1)) / 2;
            assoc_h2w[center].insert(x);
        }
    }
    auto ret = new vector<simple_line>(2000);
    ret->clear();
    for (auto itt = assoc_h2w.begin(); itt != assoc_h2w.end(); itt++) {
        auto y = itt->first;
        auto foreach = [&itt](function<void(int)> cb){
            for (auto jtt = itt->second.begin(); jtt != itt->second.end(); jtt++) {
                cb(*jtt);
            }
        };
        auto islands = ContinuousNumber::islands(foreach);
        for (int i = 0; i < islands.size(); i++) {
            auto start = islands[i][0];
            auto len = islands[i].size();
            simple_line sline;
            sline.first_dim     = y;
            sline.second_dim     = start;
            sline.len             = len;
            ret->push_back(sline);
        }
    }
    return ret;
}

vector<simple_line>* sline_trim(vector<simple_line> &lines,
        unsigned int first_dim_len, unsigned int second_dim_len,
        function<unsigned int(unsigned int, unsigned int)> get_color)
{
    map<int, map<int, simple_line>> assoc;
    auto margin0 = first_dim_len >> 4;
    auto margin1 = first_dim_len - margin0;
    auto black_threshold = second_dim_len / 20;
    for (auto itt = lines.begin(); itt != lines.end(); itt ++) {
        assoc[itt->first_dim][itt->second_dim] = *itt;
    }

    map<int, map<int, simple_line*>> trimed;

    auto len_threshold = second_dim_len / 50;
    auto merged_len_threshold = second_dim_len / 20;
    for (auto itt = assoc.begin(); itt != assoc.end(); itt ++) {
        vector<simple_line*> sorted(itt->second.size());
        sorted.clear();
        for (auto jtt = itt->second.begin(); jtt != itt->second.end(); jtt ++) {
            sorted.push_back(&jtt->second);
        }

        auto last = sorted[0];
        for (int i = 0; i < sorted.size(); i ++) {
            int gap = (int)sorted[i]->second_dim - (int)(last->second_dim + last->len);
            if (gap < sorted[i]->len / 2) {
                last->len = sorted[i]->second_dim + sorted[i]->len - last->second_dim;
            } else {
                if (last->len >= merged_len_threshold) {
                    trimed[last->first_dim][last->second_dim] = last;
                }
                last = sorted[i];
            }
        }
        if (last->len >= merged_len_threshold) {
            trimed[last->first_dim][last->second_dim] = last;
        }
    }

    auto ret = new vector<simple_line>(lines.size());
    ret->clear();
    for (auto itt = trimed.begin(); itt != trimed.end(); itt ++) {
        for (auto jtt = itt->second.begin(); jtt != itt->second.end(); jtt ++) {
            ret->push_back(*jtt->second);
        }
    }
    return ret;
}

#define MARGIN_RATIO (0.03)
static shared_ptr<vector<simple_line>>
detect_lines(unsigned int dim0, unsigned int dim1,
    function<unsigned int (unsigned int, unsigned int)> get_color,
    bool trim_short_line)
{
    unsigned int margin = (unsigned int) ((float)dim0 * MARGIN_RATIO);
    margin = max(4, (int)margin);
    int end = dim0 - margin;
    int end1 = dim1 - margin;
    int short_line_len = 10;
    auto vecptr = new vector<simple_line>(min(10000, (int)(dim0 * dim1 / 20)));
    auto vec = shared_ptr<vector<simple_line>>(vecptr);
    vec->clear();

    for (int i = margin; i < end; i++) {
        for (int j = margin; j < end1; j++) {
            if (unlikely(IS_BLACK(get_color, i, j))) {
                int start = j;
                int nr_exception = 1;
//                for (;j < end1 && nr_exception; j++) {
//                    if (!is_black(get_color, i, j)) {
//                        nr_exception --;
//                    }
//                }
                for (;j < end1 && is_black(get_color, i, j); j++)
                    /*do nothing*/;
                int len = j - start;
                if (unlikely(len > SHORT_LINE_LEN_MIN)) {
                    struct simple_line line;
                    line.first_dim = i;
                    line.second_dim = start;
                    line.len = len;
                    vec->push_back(line);
                }
            }
        }
    }
    if (trim_short_line) {
        auto trimed = sline_trim(*vec, dim0, dim1, get_color);
        vec = shared_ptr<vector<simple_line>>(trimed);
    }
    return vec;
}

void
erase_black_margin(Image &image)
{
    auto pixels = image.getPixels(0, 0, image.columns(), image.rows());
    auto width = image.columns();
    auto height = image.rows();
    auto ch = image.channels();
    auto get_color = [pixels, width, ch](unsigned int i, unsigned int j){
        unsigned offset = ch * (width * j + i);
        // auto ptr = pixels + j * width + i;
        Quantum q = min(min(pixels[offset+0], pixels[offset+2]), pixels[offset+1]);
        return ((unsigned int)q) & 0xff;
    };

    Color white(65535, 65535, 65535);
    auto set_color = [pixels, width, &white, ch](unsigned int i, unsigned int j){
        // auto ptr = pixels + j * width + i;
        unsigned offset = ch * (width * j + i);
        pixels[offset+0] = white.quantumRed();
        pixels[offset+2] = white.quantumBlue();
        pixels[offset+1] = white.quantumGreen();
    };


    unsigned int threshold = 128;
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            if (get_color(i, j) >= threshold) {
                break;
            }
            set_color(i, j);
//            image.pixelColor(i, j, white);
        }

        for (int j = height - 1; j >= 0; j--) {
            if (get_color(i, j) >= threshold) {
                break;
            }
            set_color(i, j);
//            image.pixelColor(i, j, white);
        }
    }

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            if (get_color(i, j) >= threshold) {
                break;
            }
            set_color(i, j);
//            image.pixelColor(i, j, white);
        }

        for (int i = width - 1; i >= 0; i--) {
            if (get_color(i, j) >= threshold) {
                break;
            }
            set_color(i, j);
//            image.pixelColor(i, j, white);
        }
    }
    image.syncPixels();
}

/**
  * 提取图片里水平方向的直线
  */
extern shared_ptr<vector<simple_line>>
extract_hlines(Image &image, bool trim_short_line)
{
    auto pixels = image.getConstPixels(0, 0, image.columns(), image.rows());
    int width = image.columns();
    auto ch = image.channels();
    auto get_color = [pixels, width, ch](unsigned int i, unsigned int j){
        unsigned offset = ch * (width * i + j);
        Quantum q = min(min(pixels[offset+0], pixels[offset+2]), pixels[offset+1]);
        // __typeof__(pixels) ptr = pixels + i * width + j;
        // Quantum q = min(min(ptr->red, ptr->blue), ptr->green);
        return (unsigned int)q;
    };

    return detect_lines(image.rows(), image.columns(), get_color, trim_short_line);
}

/**
  * 提取图片里垂直方向的直线
  */
extern shared_ptr<vector<simple_line>>
extract_vlines(Image &image, bool trim_short_line)
{
    auto pixels = image.getConstPixels(0, 0, image.columns(), image.rows());
    int width = image.columns();
    auto ch = image.channels();
    auto get_color = [pixels, width, ch](unsigned int i, unsigned int j){
        unsigned offset = ch * (width * j + i);
        Quantum q = min(min(pixels[offset+0], pixels[offset+2]), pixels[offset+1]);
    // __typeof__(pixels) ptr = pixels + j * width + i;
        // Quantum q = min(min(ptr->red, ptr->blue), ptr->green);
        return (unsigned int)q;
    };

    return detect_lines(image.columns(), image.rows() , get_color, trim_short_line);
}

/**
  * 计算一组线条在直线方向上的权值
  */
extern unsigned long calc_weight(vector<struct simple_line> &lines,
        unsigned int first_dim_len, unsigned int second_dim_len, int offset)
{
    vector<int> weight(first_dim_len);
    fill_n(weight.begin(), first_dim_len, 0);
    int step = 20;
    for (int i = 0; i < lines.size(); i++) {

        for (int j = 0; j < lines[i].len; j+= step) {
            int delta = (int)(lines[i].second_dim + j) * offset / (int)second_dim_len;
            unsigned int index = lines[i].first_dim + delta;
            if (index < first_dim_len) {
                weight[index] += step;
            }
        }
    }

    unsigned long ret = 0;
    for (int i = 1; i < weight.size(); i++) {
        unsigned long w = weight[i];
        ret += w * w;
    }

    return ret;
}

#define RANGE (200)
extern int simple_line_offset(std::vector<struct simple_line> &lines,
        unsigned int first_dim_len, unsigned int second_dim_len)
{
    int offset = 0;
    unsigned long old_weight = 0;

    for (int i = - RANGE; i <= RANGE; i++) {
        unsigned long total_weight =
            calc_weight(lines, first_dim_len, second_dim_len, i);
        if (total_weight > old_weight) {
            old_weight = total_weight;
            offset = i;
        }
    }
    return offset;
}

extern std::vector<simple_line>
merge_simple_lines(std::vector<simple_line> &lines,
        unsigned int first_dim_len, unsigned int second_dim_len, int offset)
{
    vector<simple_line> ret(lines.size());
    ret.clear();
    for (int i = 0; i < lines.size(); i++) {
        int delta = (int)lines[i].second_dim * offset / (int)second_dim_len;
        unsigned int index = lines[i].first_dim + delta;
        if (index < first_dim_len) {
            simple_line l;
            l.first_dim = index;
            l.second_dim = lines[i].second_dim;
            l.len = lines[i].len;
            ret.push_back(l);
        }
    }
    return ret;
}

extern Image deskew(Image &img, int v_offset, int h_offset)
{
    size_t width = img.columns();
    size_t height = img.rows();
    double sx = 1.0;
    double sy = 1.0;
    double tx = 0.0;
    double ty = 0.0;
    double rx = (double) h_offset / (double) width;
    double ry = (double) v_offset / (double) height;
    DrawableAffine affine(sx, sy, rx, ry, tx, ty);
    img.affineTransform(affine);
    return img;
}

/**
  * 给定一组直线，生成这些直线为谷底的等势图
  */
extern unordered_map<int, IntDefaulted>
potential_map(vector<simple_line> &lines, int second_dim_len)
{
    unordered_map<int, IntDefaulted> pmap;
    auto len = lines.size();
    for (int i = 0; i < len; i ++) {
        auto line = &lines[i];
        int start = line->second_dim - PMAP_DELTA * 2;
        start = max(0, start);
        int end = line->second_dim + line->len + PMAP_DELTA * 2;
        for (int j = start; j < end; j ++) {
            for (int k = max(0, (int)(line->first_dim - PMAP_DELTA));
                    k < line->first_dim + PMAP_DELTA; k++) {
                int key = k * second_dim_len + j;
                unsigned int d = k > line->first_dim ?
                    k - line->first_dim : line->first_dim - k;
                if (pmap[key] > d) {
                    pmap[key] = d;
                }
            }
        }
    }
    return pmap;
}

PotentialBitMap::PotentialBitMap(vector<simple_line> &lines)
{
    pmap = potential_map(lines, 10000);
}

unsigned int
PotentialBitMap::get(int x, int y)
{
    return pmap[y * 10000 + x];
}

extern unsigned long
PotentialMap::lines_diff(
        vector<simple_line> &lines,
        unsigned long init,
        function<void(unsigned long)> hook,
        function<void(int&, int&)> transform)
{
    unsigned long diff = init;
    unsigned long step = SIMPLE_LINE_SAMPLE_STEP;
    for (auto line = lines.begin(); line != lines.end(); line ++) {
        for (int j = line->second_dim; j < line->len + line->second_dim; j += step) {
            int x = j;
            int y = line->first_dim;
            transform(x, y);
            int d = this->get(x, y);
//            diff += d * d * step;
            d = d > 0 ? d : -d;
            diff += d * step;
            hook(diff);
        }
    }
    return diff;
}

extern unsigned long
potential_map_lines_diff(
        unordered_map<int, IntDefaulted> &pmap,
        vector<simple_line> &lines,
        unsigned long init,
        function<void(unsigned long)> hook,
        function<int(int, int)> get_key)
{
    unsigned long diff = init;
    unsigned long step = SIMPLE_LINE_SAMPLE_STEP;
    for (auto line = lines.begin(); line != lines.end(); line ++) {
        for (int j = line->second_dim; j < line->len + line->second_dim; j += step) {
            int d = pmap[get_key(line->first_dim, j)];
            diff += d * d * step;
            hook(diff);
        }
    }
    return diff;
}

extern unsigned long
potential_map_lines_diff(
        unordered_map<int, IntDefaulted> &pmap,
        vector<simple_line> &lines,
        unsigned long init,
        function<void(unsigned long)> hook)
{
    auto get_key = [](int x, int y) {
        return x * 10000 + y;
    };
    return potential_map_lines_diff(pmap, lines, init, hook, get_key);
}

extern unsigned long
potential_map_lines_diff(
        unordered_map<int, IntDefaulted> &pmap,
        vector<simple_line> &lines,
        unsigned long init)
{
    auto hook = [](unsigned long){};
    auto get_key = [](int x, int y) {
        return x * 10000 + y;
    };
    return potential_map_lines_diff(pmap, lines, init, hook, get_key);
}


struct no_need_to_run : public std::exception
{
};

#define PMAP_SECOND_DIM_LEN 10000

simple_lines_affine_distance
simple_lines_affine_diff(
        std::vector<simple_line> &to,
        std::vector<simple_line> &from,
        unsigned int from_len,
        unsigned int t_limit,
        unsigned int s_limit,
        unsigned long init,
        function<void(unsigned long)> cb)
{
    unsigned long diff = ULONG_MAX;
    vector<int> scale(s_limit * 2);

    {
        /**
          * 候选的缩放值
          */
        scale.clear();
        scale.push_back(0);
        for (int i = 1; i < s_limit; i++) {
            scale.push_back(i);
            scale.push_back(-i);
        }
    }

    vector<int> transform(t_limit * 2);

    {
        /**
          * 候选的平移值
          */
        transform.clear();
        transform.push_back(0);
        for (int i = 1; i < t_limit; i++) {
            transform.push_back(i);
            transform.push_back(-i);
        }
    }

#ifdef USE_DEPRECATED_POTENTIAL_BIT_MAP
    auto to_pmap = potential_map(to, PMAP_SECOND_DIM_LEN);
    auto from_pmap = potential_map(from, PMAP_SECOND_DIM_LEN);
#else
    PotentialMap to_pm(to, from_len);
    PotentialMap from_pm(from, from_len);
#endif
    simple_lines_affine_distance d = {
        .t = 0,
        .s = 1.0,
        .diff = ULONG_MAX,
    };
    if (!from_len) {
        return d;
    }


    auto hook = [&d, cb](unsigned long diff){
        cb(diff);
        if (diff >= d.diff) {
            throw no_need_to_run();
        }
    };

    /**
      * 以to_pmap为等势图
      */
    for (int i = 0; i < scale.size(); i ++) {
        int s = scale[i];
        unsigned int new_from_len = from_len + s;
        new_from_len = new_from_len ?: 1;
        for (int j = 0; j < transform.size(); j++) {
            int t = transform[j];
            auto get_key = [new_from_len, t, from_len](int x, int y){
                return (x * new_from_len / from_len + t) * PMAP_SECOND_DIM_LEN + y;
            };
            auto to_transform = [new_from_len, t, from_len](int &x, int &y){
                y = y * new_from_len / from_len + t;
            };
            auto get_from_pmap_key = [new_from_len, t, from_len](int x, int y){
                return (x * from_len / new_from_len + t) * PMAP_SECOND_DIM_LEN + y;
            };
            auto from_transform = [new_from_len, t, from_len](int &x, int &y){
                y = y * from_len / new_from_len + t;
            };
            try {
#ifdef USE_DEPRECATED_POTENTIAL_BIT_MAP
                unsigned long to_diff =
                    potential_map_lines_diff(to_pmap, from, init, hook, get_key);
                unsigned long from_diff =
                    potential_map_lines_diff(from_pmap, to, to_diff, hook, get_from_pmap_key);
#else
                unsigned long to_diff =
                    to_pm.lines_diff(from, init, hook, to_transform);
                unsigned long from_diff =
                    from_pm.lines_diff(to, to_diff, hook, from_transform);
#endif
                d.diff = from_diff;
                d.t = t;
                d.s = (float)new_from_len / (float)from_len;
                if (d.diff == 0) {
                    return d;
                }
            } catch (no_need_to_run &e) {
            }
        }
    }

    return d;
}

void
SLineImage::init_data()
{
    vector<unsigned int> data(first_dim_len + 1);
    fill(data.begin(), data.end(), 0);
    for (auto itt = slines->begin(); itt != slines->end(); itt ++) {
        data[itt->first_dim] += itt->len;
    }
    unsigned int threshold = second_dim_len / 10;
    get_data_peaks(data, first_dim_peaks, first_dim_peak_weight, threshold);
    first_dim_peak_neighbor.init_data(first_dim_peaks);
//    for (auto itt = first_dim_peaks.begin(); itt != first_dim_peaks.end(); itt ++) {
//        cout << *itt << "\t" << first_dim_peak_weight[*itt] << endl;
//    }
//    cout << endl << endl;
}

simple_lines_affine_distance
SLineImage::diff(
        SLineImage &target,
        unsigned int t_limit,
        unsigned int s_limit,
        unsigned long init)
{
    std::vector<simple_line> &to = *target.slines;
    std::vector<simple_line> &from = *this->slines;
    unsigned int from_len = this->first_dim_len;

    unsigned long diff = ULONG_MAX;
    vector<int> scale(s_limit * 2);

    {
        /**
          * 候选的缩放值
          */
        scale.clear();
        scale.push_back(0);
        for (int i = 1; i < s_limit; i+=3) {
            scale.push_back(i);
            scale.push_back(-i);
        }
    }

    vector<int> transform(t_limit * 2);

    {
        /**
          * 候选的平移值
          */
        transform.clear();
        transform.push_back(0);
        for (int i = 1; i < t_limit; i++) {
            transform.push_back(i);
            transform.push_back(-i);
        }
    }

    PotentialMap &to_pm = target.pmap;
    PotentialMap &from_pm = this->pmap;

    simple_lines_affine_distance d = {
        .t = 0,
        .s = 1.0,
        .diff = ULONG_MAX,
    };
    if (!from_len) {
        return d;
    }

    auto hook = [&d](unsigned long diff){
        if (diff > d.diff) {
            throw no_need_to_run();
        }
    };

    float pruning_threshold = 0.95;

    auto foreach_point = [this, &target, &t_limit, &pruning_threshold]
        (function<void(unsigned int, unsigned int, float)> cb) {
        t_limit = 200;
        for (auto itt = this->first_dim_peaks.begin();
                itt != this->first_dim_peaks.end(); itt++) {
            for (auto jtt = target.first_dim_peaks.begin();
                    jtt != target.first_dim_peaks.end(); jtt++) {
                auto from_index = *itt;
                auto from_weight = this->first_dim_peak_weight[from_index];
                auto to_index = *jtt;
                auto to_weight = target.first_dim_peak_weight[to_index];

                int d = from_index - to_index;
                d = (d > 0 ? d : -d);
                if (d > t_limit) continue;

                float from_ratio = (float) from_weight / (from_weight + to_weight);
                float to_ratio = (float) to_weight / (from_weight + to_weight);

                /**
                  * 这里使用信息熵来刻画相似度，相同为1
                  */
                float similar = - (from_ratio * log(from_ratio) + to_ratio * log(to_ratio))
                        / log(2);
                if (similar < pruning_threshold) continue;

                cb(from_index, to_index, similar);
            }
        }
    };

    vector<point_t> points(100);
    map<point_t, float> similarity;
    points.clear();
    similarity.clear();

    vector<float> pruning_threshold_cases{0.97, 0.95, 0.9, 0.85, 0.8, 0.7};
    for (int i = 0; i < pruning_threshold_cases.size(); i++) {
        if (points.size() >= 100) {
            break;
        }
        pruning_threshold = pruning_threshold_cases[i];
        points.clear();
        similarity.clear();
        foreach_point([&points, &similarity](unsigned int from, unsigned int to, float similar){
            auto point = tie(from, to);
            points.push_back(point);
            similarity[point] = similar;
        });
    }

    auto foreach_transform_fast =
    [&points, t_limit, s_limit, &transform, &similarity, &target, this]
    (function<void(int, float)> cb){
        set<tuple<int, int>> transforms;
        for (int i = 0; i < points.size(); i++) {
            for (int j = i + 1; j < points.size(); j++) {
                int from0, to0, from1, to1;
                tie(from0, to0) = points[i];
                tie(from1, to1) = points[j];
                if (from0 == from1 || to0 == to1) continue;
                float s = (float)(to1 - to0) / (from1 - from0);
                int t = s * (0 - from0) + to0;
                int abs_t = t > 0 ? t : -t;
                if (abs_t > t_limit) continue;
                /**
                 * 两张图缩放比例不能相差太远
                 */
                if (s > 1.2 || s < 0.85) continue;
                int s_int = (int)(1000 * s);
                transforms.insert(tie(t, s_int));
            }
        }

        map<tuple<int, int>, float> point_confidence;
        for (auto itt = transforms.begin(); itt != transforms.end(); itt++) {
            int t, s_int;
            tie(t, s_int) = *itt;
            float s = (float)s_int / 1000;
            /**
              * 如果 y = sx + t 这条直线的置信度confidence较大，
              * 说明 直线上串的点非常多
              */
            float confidence = get_line_confidence(
                this->first_dim_peaks, this->first_dim_peak_neighbor,
                target.first_dim_peaks, target.first_dim_peak_neighbor,
                similarity, s, t
            );
            point_confidence[*itt] = confidence;
        }

        vector<tuple<int, int>> vec_trans(transforms.size());
//        cerr << "nr-s-t-case=" << vec_trans.size() << endl;
        vec_trans.clear();
        for (auto itt = transforms.begin(); itt != transforms.end(); itt++) {
            auto t = *itt;
            vec_trans.push_back(t);
        }

        /**
          * 按置信度排序，方便最大程度的剪枝
          */
        sort(vec_trans.begin(), vec_trans.end(),
        [&point_confidence](const tuple<int, int> &a, const tuple<int, int> &b){
            return point_confidence[a] > point_confidence[b];
        });

        int count = 0;
        int tryCount = 100;
        for ( auto itt = vec_trans.begin();
                itt != vec_trans.end(); itt++, count++) {
            int t, s_int;
            tie(t, s_int) = *itt;
            float s = (float)s_int / 1000;
            if (count < tryCount) {
                cb(t, s);
            }
        }
    };

    auto foreach_transform = [&scale, &transform, from_len](
            function<void(int t, float s)> cb){
        for (int i = 0; i < scale.size(); i ++) {
            int s_offset = scale[i];
            unsigned int new_from_len = from_len + s_offset;
            new_from_len = new_from_len ?: 1;
            float s = (float)new_from_len / (float)from_len;
            for (int j = 0; j < transform.size(); j++) {
                int t = transform[j];
                cb(t, s);
            }
        }
    };

    auto do_each_transform =
        [&to_pm, &from_pm, &from, &to, &init, &hook, &d](int t, float s){
            auto to_transform = [s, t](int &x, int &y){
                y = (float)y * s + t;
            };
            auto from_transform = [s, t](int &x, int &y){
                y = ((float)y - t) / s;
            };
            /**
              * 以to_pmap为等势图
              */
            try {
//                cerr << "t=" << t << ", s=" << s << endl;
                unsigned long to_diff =
                    to_pm.lines_diff(from, init, hook, to_transform);
                unsigned long from_diff =
                    from_pm.lines_diff(to, to_diff, hook, from_transform);
                if (from_diff <= d.diff) {
                    d.diff = from_diff;
                    d.t = t;
                    d.s = s;
                }
//                cerr << "t=" << d.t << ", s=" << s << ", diff=" << d.diff << endl;
            } catch (no_need_to_run &e) {
            }

            if (d.diff == 0) {
                throw no_need_to_run();
            }
    };

    try {
        foreach_transform_fast(do_each_transform);
    } catch (no_need_to_run &e) {
    }

    return d;
}

extern simple_lines_affine_distance
simple_lines_affine_diff(
        std::vector<simple_line> &to,
        std::vector<simple_line> &from,
        unsigned int from_len,
        unsigned int t_limit,
        unsigned int s_limit,
        unsigned long init)
{
    auto cb = [](unsigned long) {};
    return simple_lines_affine_diff(to, from, from_len, t_limit, s_limit, init, cb);
}


simple_line::operator string()
{
    stringstream ss;
    ss << "1st=" << this->first_dim << ", 2nd=" << this->second_dim << ", len=" << this->len;
    return ss.str();
}


ImageLine::operator string()
{
    stringstream ss;
    ss << "horizontal-length: " << height << endl;
    for (int i = 0; i < horizontal->slines->size(); i++) {
        ss << (string)(*horizontal->slines)[i] << endl;
    }
    ss << "vertical-length: " << width << endl;
    for (int i = 0; i < vertical->slines->size(); i++) {
        ss << (string)(*vertical->slines)[i] << endl;
    }
    return ss.str();
}

static unsigned int
slines_roundup_len(vector<simple_line> &lines)
{
    unsigned int ret = 0;
    for (auto itt = lines.begin(); itt != lines.end(); itt++) {
        ret += (itt->len + SIMPLE_LINE_SAMPLE_STEP - 1)
            / SIMPLE_LINE_SAMPLE_STEP * SIMPLE_LINE_SAMPLE_STEP;
    }
    return ret;
}

ImageLineAffineDistance
ImageLine::distance(ImageLine &target,
        unsigned int t_limit, unsigned int s_limit)
{

    auto sline_vdistance =
        vertical->diff(*target.vertical, t_limit, s_limit, 0);

    auto sline_hdistance = horizontal->diff(
                *target.horizontal, t_limit, s_limit, 0);

    ImageLineAffineDistance distance;
    distance.tx = sline_vdistance.t;
    distance.sx = sline_vdistance.s;

    distance.ty = sline_hdistance.t;
    distance.sy = sline_hdistance.s;

//    cout << "vertical-diff=" << sline_vdistance.diff
//        << ", horizontal-diff=" << sline_hdistance.diff << endl;
    float hdiff, vdiff;
    float h_size = slines_roundup_len(*horizontal->slines) ?: 1;
    float v_size = slines_roundup_len(*vertical->slines) ?: 1;
    hdiff = (float)sline_hdistance.diff / h_size;
    vdiff = (float)sline_vdistance.diff / v_size;
//    cout << "vdiff=" << vdiff << ", hdiff=" << hdiff << endl;

    distance.diff = (hdiff + vdiff) / 2;
    return distance;
}

unsigned int
ImageLine::roundup_len()
{
    return slines_roundup_len(*vertical->slines) +
        slines_roundup_len(*horizontal->slines);
}

ImageLine::ImageLine(Image &img)
{
    width = img.columns();
    height = img.rows();
    auto hl = extract_hlines(img, true);
    shared_ptr<vector<simple_line>> hlines(sline_thin(*hl));

//    horizontal = shared_ptr<SLineImage>(new SLineImage(hl, height, width));
    horizontal = shared_ptr<SLineImage>(new SLineImage(hlines, height, width));

    auto vl = extract_vlines(img, true);
    shared_ptr<vector<simple_line>> vlines(sline_thin(*vl));

//    vertical = shared_ptr<SLineImage>(new SLineImage(vl, width, height));
    vertical = shared_ptr<SLineImage>(new SLineImage(vlines, width, height));
}

ImageLineAffineDistance::operator string()
{
    stringstream ss;
    ss     << "diff=" << diff << ", r=" << rotation
        << ", tx=" << tx << ", ty=" << ty
        << ", sx=" << sx << ", sy=" << sy
        << ", f=" << filename << ", t=" << templateFileName
        ;
    return ss.str();
}

void PotentialMap::init_data(vector<simple_line> &lines, unsigned int first_dim_len)
{
    this->first_dim_len = first_dim_len;
    auto get_start = [](simple_line &line){
        int start = line.second_dim - PMAP_DELTA * 2;
        return start;
    };
    auto get_end = [](simple_line &line){
        int end = line.second_dim + line.len + PMAP_DELTA * 2;
        return end;
    };

    map<int, set<int>> assoc_w2h;
    for (int i = 0; i < lines.size(); i++) {
        auto start = get_start(lines[i]);
        auto end = get_end(lines[i]);
        for (int j = start; j < end; j++) {
            assoc_w2h[j].insert(lines[i].first_dim);
        }
    }
    for (auto itt = assoc_w2h.begin(); itt != assoc_w2h.end(); itt++) {
        vector<int> vec(itt->second.size());
        vec.clear();
        for (auto jtt = itt->second.begin(); jtt != itt->second.end(); jtt++) {
            vec.push_back(*jtt);
        }
        map<int, Integer<-100000>> left;
        map<int, Integer<100000>> right;
        for (int i = 0; i + 1 < vec.size(); i++) {
            auto start = vec[i];
            auto end = vec[i + 1];
            auto middle = (start + end) / 2;
            left[end] = middle;
            right[start] = middle;
        }
        for (int i = 0; i < vec.size(); i++) {
            auto point = vec[i];
            assoc_neighbor[itt->first][tie(left[point], right[point])] = point;
        }
    }

}

unsigned int PotentialMap::get(int x, int y)
{
    int neighbor = assoc_neighbor[x][make_tuple(y, y)];
    int d = neighbor - y;
    d = d > 0 ? d : -d;
    return min(PMAP_DELTA, d);
}

PotentialMap::PotentialMap(vector<simple_line> &lines, unsigned int first_dim_len)
{
    init_data(lines, first_dim_len);
}

RotableImage::RotableImage(string filename) :
    filename(filename)
{
    Image image(filename);

    unsigned int rotations[] = { 0, 90, 180, 270 };
    unsigned int size_of_rotations = sizeof(rotations) / sizeof(rotations[0]);
    for (int i = 0; i < size_of_rotations; i ++) {
        auto r = rotations[i];
        if (r) {
            image.rotate(90);
        }
        auto il = new ImageLine(image);
        image_lines[r] = shared_ptr<ImageLine>(il);
    }
}

CandidateImages::CandidateImages(vector<string> filenames)
{
    candidates.resize(filenames.size());
    candidates.clear();
    for (auto itt = filenames.begin(); itt != filenames.end(); itt ++) {
        candidates.push_back(RotableImage(*itt));
    }
}

CandidateImages::CandidateImages(const initializer_list<string> &filenames)
{
    candidates.clear();
    for (auto &filename : filenames) {
        candidates.push_back(RotableImage(filename));
    }
}

void
CandidateImages::for_each(function<void(const string&, int, ImageLine&)> cb)
{
    for (auto itt = candidates.begin(); itt != candidates.end(); itt++) {
        for (auto jtt = itt->image_lines.begin(); jtt != itt->image_lines.end(); jtt++) {
            int r = jtt->first;
            cb(itt->filename, r, *jtt->second);
        }
    }
}


ImageLineAffineDistance
CandidateImages::nearest(string filename, unsigned int t_limit, unsigned int s_limit)
{
    Image img(filename);
    auto d = nearest(img, t_limit, s_limit);
    d.filename = filename;
    return d;
}

void
CandidateImages::drawTemplateLine(Image &image, ImageLineAffineDistance &d)
{
    auto rotation = 360 - d.rotation;

    Image img(image);
    Color black(0, 0, 0);
    img.rotate(rotation);
    RotableImage *ri = NULL;
    for (auto itt = candidates.begin(); itt != candidates.end(); itt++) {
        if (itt->filename == d.templateFileName) {
            ri = & *itt;
        }
    }

    auto il = ri->image_lines[0];
    for (auto itt = il->horizontal->slines->begin();
            itt != il->horizontal->slines->end(); itt ++) {
        unsigned int y = (unsigned int)(((float)itt->first_dim - d.ty) / d.sy);
        for (int j = 0; j < itt->len; j++) {
            unsigned int x = (unsigned int)(((float)j + itt->second_dim - d.tx) / d.sx);
//            unsigned int x = j + itt->second_dim;
            if (x < img.columns() && y < img.rows()) {
                img.pixelColor(x, y, black);
            }
        }
    }
    for (auto itt = il->vertical->slines->begin();
            itt != il->vertical->slines->end(); itt ++) {
        unsigned int x = (unsigned int)(((float)itt->first_dim - d.tx) / d.sx);
        for (int j = 0; j < itt->len; j++) {
            unsigned int y = (unsigned int)(((float)j + itt->second_dim - d.ty) / d.sy);
//            unsigned int y = j + itt->second_dim;
            if (x < img.columns() && y < img.rows()) {
                img.pixelColor(x, y, black);
            }
        }
    }
    img.write("/home/yjf/rotated.jpg");
}

ImageLineAffineDistance
CandidateImages::nearest(Image &img, unsigned int t_limit, unsigned int s_limit)
{
    ImageLine source(img);
    ImageLineAffineDistance best;
    best.diff = (float)ULONG_MAX;

    int best_rotation = 0;
    string best_filename;

    this->for_each([this, &source, t_limit, &best,
            &best_rotation, &best_filename]
            (const string &filename, int rotation, ImageLine& rotated) {
        if (!config.full_rotated
            && ((source.width >= source.height) ^ (rotated.width >= rotated.height))) {
            /**
              * 假定图片是长方形，且没有形变
              * 注意异或结果：相同得0，不同得1
              */
            return;
        }
        auto rotated_diff = source.distance(rotated, t_limit, 0);
//        cout << "diff=" << rotated_diff.diff
//            << ", rotation=" << rotation << ", filename=" << filename << endl;
        if (rotated_diff.diff <= best.diff) {
            best = rotated_diff;
            best.rotation = best_rotation = rotation;
            best.templateFileName = best_filename = filename;
        }
    });

//    return best;

    /**
      * 理论上来讲，上边代码能确定图片的仿射参数了，
      * 但由于毕竟不是在旋转待分类图片，
      * 实际所计算的sx,sy,tx,ty是会有少许误差的，为了将误差降到最低，
      * 我们补作一次旋转定位
      */
    if (best_rotation) {
        img.rotate(360 - best_rotation);
        source = ImageLine(img);
        this->for_each([this, &source, t_limit, &best_filename, &best]
                    (const string &filename, int rotation, ImageLine& rotated) {
            if (filename == best_filename && rotation == 0) {
                best = source.distance(rotated, t_limit, 0);
            }
        });
    }
    best.rotation = best_rotation;
    best.templateFileName = best_filename;
    best.diff = (best.diff < 888888 ? best.diff : 888888);
    return best;
}

string extract_black_rectangle(string img_base64, int threshold = 128)
{
    Blob b;
    b.base64(img_base64);
    Rectangle rect(0, 0, 1, 1);
    unsigned int count = 0;
    unsigned int err = 0;
    try {
        Image img(b);
        tie(rect, count) = biggest_black_rect(img, threshold);
    } catch (exception &e) {
        err = 1;
    }
    Json::Value obj;
    obj["error"] = err;
    obj["x"] = rect.x;
    obj["y"] = rect.y;
    obj["w"] = rect.w;
    obj["h"] = rect.h;
    obj["nr-black-pixel"] = count;
    return json_encode(obj);
}

string
classify_by_line(string templates, string images)
{
//    cout << "template-json:\n" << templates << endl;
//    cout << "image-json:\n" << images << endl;
    auto vecTemplateFilename = json2vector(templates);
    auto vecImageFileName = json2vector(images);
    auto candidateImages = CandidateImages(vecTemplateFilename);
    Json::Value root(Json::arrayValue);
    for (auto itt = vecImageFileName.begin(); itt != vecImageFileName.end(); itt++) {
        try {
            auto distance = candidateImages.nearest(*itt);
//            cout << *itt << endl;
//            cout << (string) distance << endl;
            Json::Value item;
            item["diff"] = distance.diff;
            item["k"] = distance.sx;
            item["sx"] = distance.sx;
            item["sy"] = distance.sy;
            item["bw"] = distance.tx;
            item["bh"] = distance.ty;
            item["tx"] = distance.tx;
            item["ty"] = distance.ty;
            item["rotation"] = distance.rotation;
            item["template_name"] = distance.templateFileName;
            item["name"] = distance.filename;
            root.append(item);
        } catch (exception &e) {
        }
    }
    auto ret = json_encode(root);
    return ret;
}


inline unsigned long
pack_rectangle(int x, int y, int w, int h)
{
        return (((unsigned long)x * 10000 + (unsigned long)y) * 10000 + (unsigned long)w) * 10000 + (unsigned long)h;
}

inline Rectangle
unpack_rectangle(unsigned long v)
{
    int h = v % 10000;
    v /= 10000;
    int w = v % 10000;
    v /= 10000;
    int y = v % 10000;
    v /= 10000;
    int x = v % 10000;
    return Rectangle(x, y, w, h);
};

tuple<Rectangle, unsigned int>
biggest_black_rect(Image &image,
        unsigned int threshold, unsigned int w_limit, unsigned int h_limit)
{
    unsigned int width = image.columns();
    unsigned int height = image.rows();
    unordered_map<unsigned long, unsigned int> assoc(width * height * width * height);

    auto pixels = image.getConstPixels(0, 0, image.columns(), image.rows());
    auto ch = image.channels();
    auto get_color = [pixels, width, ch](unsigned int i, unsigned int j){
        // __typeof__(pixels) ptr = pixels + j * width + i;
	unsigned offset = ch * (width * j + i);
        auto r = (int)pixels[offset+0] & 0xff;
        auto g = (int)pixels[offset+2] & 0xff;
        auto b = (int)pixels[offset+1] & 0xff;
        return (unsigned int) (r * 299 + g * 587 + b * 114) / 1000;

    };

    float total = 0;
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            auto color = get_color(i, j);
            int x = color < threshold ? 1 : 0;
            assoc[pack_rectangle(i, j, 1, 1)] = x;
            total += x;
        }
    }

//    cout << "w-limit=" << w_limit <<
//        ", h-limit=" << h_limit <<
//        ", width=" << width <<
//        ", height=" << height << endl;


    /**
     * 动态规划法求每个矩形的黑点个数
     */
    w_limit = w_limit > 1 ? w_limit : 1;
    w_limit = w_limit < width ? w_limit : width;
    h_limit = h_limit > 1 ? h_limit : 1;
    h_limit = h_limit < height ? h_limit : height;

    for (int w = 1; w <= w_limit; w++) {
        for (int h = 1; h <= h_limit; h++) {
            if (w == 1 && h == 1) continue;
            for (int i = 0; i + w <= width; i++) {
                for (int j = 0; j + h <= height; j++) {
                    auto _w = w - 1;
                    auto _h = h - 1;
                    auto x = i + w - 1;
                    auto y = j + h - 1;
                    assoc[pack_rectangle(i, j, w, h)] =
                        (_w && _h ? assoc[pack_rectangle(i, j, _w, _h)] : 0) +
                        (_h ? assoc[pack_rectangle(x, j, 1, _h)] : 0) +
                        (_w ? assoc[pack_rectangle(i, y, _w, 1)] : 0) +
                        assoc[pack_rectangle(x, y, 1, 1)]
                        ;
                }
            }
        }
    }

    float size = (width + 1) * (height + 1);

    auto biggest = Rectangle(0, 0, width, height);
    unsigned biggest_count = 0;
    float g = 0;
    for (auto itt = assoc.begin(); itt != assoc.end(); itt ++) {
        float count = itt->second;
        auto rect = unpack_rectangle(itt->first);
        float mysize = rect.w * rect.h;
        auto diff = (count / mysize) - ((total - count) / (size - mysize));
        float sgn = diff > 0 ? 1 : -1;
        float tmp_g = sgn * mysize * (size - mysize) * diff * diff;
        if (tmp_g > g) {
            g = tmp_g;
            biggest = rect;
            biggest_count = count;
        }
    }
    //cout << "assoc-size=" << assoc.size() << endl;
    return tie(biggest, biggest_count);
}