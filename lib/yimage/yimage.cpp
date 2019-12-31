#include <Magick++.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <boost/foreach.hpp>
#include <climits>
#include <math.h>
#include <libgen.h>
#include "utils.hpp"
#include <boost/tuple/tuple.hpp>
// #include <boost/date_time.hpp>
#include "fn.hpp"
#include "xl.hpp"
#include <sstream>
#include "yimage.hpp"
#include <json/json.h>

#define nr_color_grade ((unsigned int) 16)

#define DIFF_DELTA 100
#define NR_SLICE 15
#define LOCAL_COMPARE 5
#define SAMPLE_SIZE 1024
#define EPSILON 0.000001F
//#define YIMAGE_DEBUG

/**
* 平移尝试次数
*/
#define PANNING_LIMIT 200

#ifdef YIMAGE_DEBUG
#define __ENTER__()                                                                                                    \
{                                                                   \
    cout << "__enter__: " << __FUNCTION__ << endl;                    \
}
#else
#define __ENTER__()
#endif

#ifdef YIMAGE_DEBUG
#define __LEAVE__()                                                                                            \
{                                                               \
    cout << "__leave__: " << __FUNCTION__ << endl;                     \
}
#else
#define __LEAVE__()
#endif


using namespace std;
using namespace Magick;
using namespace boost;
// using namespace boost::posix_time;

typedef boost::tuple<
    unsigned int,//columns
    vector<unsigned int>,//vertical lines
    unsigned int,//rows
    vector<unsigned int >//horizontal lines
> ImageVHLines;

typedef boost::tuple<string, ImageVHLines> LineFeature;

typedef vector<vector<float>> Matrix;

struct PartialImage {
    string id;
    string filename;
    Image image;
    float x;
    float y;
    float w;
    float h;
    float entropy;
};

struct PartialMatrix {
    Matrix *matrix;
    int x;
    int y;
    float w;
    float h;
    float entropy;
};

PartialMatrix PartialMatrix_new(Matrix *matrix, int x, int y, float w, float h)
{
    struct PartialMatrix pm;
    pm.matrix = matrix;
    pm.x = x;
    pm.y = y;
    pm.w = w;
    pm.h = h;
    pm.entropy = 0;
    return pm;
}

PartialMatrix PartialMatrix_new(PartialMatrix &parent, int x, int y, float w, float h)
{
    struct PartialMatrix pm;
    pm.matrix = parent.matrix;
    pm.x = parent.x + x;
    pm.y = parent.y + y;
    pm.w = w;
    pm.h = h;
    pm.entropy = 0;
    return pm;
}

vector<vector<PartialMatrix>> PartialMatrix_partition(PartialMatrix &parent, int n = 0)
{
//    __ENTER__();
    vector<vector<PartialMatrix>> ret(n);
    n = max(2, n);
    int matrixRank = parent.matrix->size();
    int subMatrixRank = matrixRank / n;
    for (int i = 0; i < n; i++) {
        vector<PartialMatrix> row(n);
        for (int j = 0; j < n; j++) {
            row[j] = PartialMatrix_new(parent,
                                    matrixRank * i / n, matrixRank * j / n,
                                    subMatrixRank, subMatrixRank);
        }
        ret[i] = row;
    }
//    __LEAVE__();
    return ret;
}

string
ImageVHLinesToString(const ImageVHLines &vhLines);

ImageVHLines
ImageLines(Image &image);

// ptime now(void)
// {
//    return second_clock::local_time();
// }


vector<unsigned int>
DetectLines(unsigned length, unsigned int from, unsigned int to, std::function<unsigned int(unsigned int, unsigned int)> getColor)
{
    vector<unsigned int> ret;
    if (from > to) {
        return ret;
    }
#ifdef YIMAGE_DEBUG
    vector<unsigned int> debug_variances(256);
    fill_n(debug_variances.begin(), 256, 0);
#endif

    vector<unsigned int> sums(length);
    vector<unsigned int> averages(length);
    vector<unsigned int> lines;

    fill_n(sums.begin(), length, 0);
    fill_n(averages.begin(), length, 0);

    unsigned int average_length = (to - from) / nr_color_grade;
    unsigned int threshold = (to - from) / 5;

    //统计各颜色值的分布
    unsigned int distro[nr_color_grade];

    int start = length / 50 + 4;
    int end = length - start;
    for (unsigned int i = start; i < end; i++) {
        unsigned int sum = 0;
        unsigned int avg = 0;
        unsigned int variance = 0;

        memset(distro, 0, sizeof(distro));

        for (unsigned int j = from; j < to; j ++) {
            unsigned int q = getColor(i, j);
            unsigned int efficient_part = q & 0xff;
            efficient_part = 0xff - efficient_part;
            unsigned int index = efficient_part * nr_color_grade / 256;
            index = min((int)index, (int) nr_color_grade - 1);
            distro[index] ++;
            if (distro[0] + distro[1] >= threshold) {
                  goto next;
            }
            sum += efficient_part;
        }

        averages[i] = avg = sum / (to - from);
        for (unsigned int k = 0; k < nr_color_grade; k++) {
              unsigned int grade = (k + 1) * 256 / nr_color_grade;
            variance += (grade - avg) * (grade - avg) * distro[k];
        }
        variance = variance / (to - from);
#ifdef YIMAGE_DEBUG
        debug_variances[min((unsigned int)sqrt(variance), (unsigned int)255)]++;
#endif

        if (variance < 32 * 32) {
            sums[i] = sum;
            lines.push_back(i);
        }
next:
    ;
    }

#ifdef YIMAGE_DEBUG
    for (int i = 0; i < 256; i++) {
      if (debug_variances[i]) {
        cout << i << "\t" << debug_variances[i] << endl;
      }
    }
#endif

    float local_compare = LOCAL_COMPARE;
    for(int i = 0; i < lines.size(); i++) {
        unsigned int x = lines[i];
        if (x > 3 && x < length - 4) {
            auto smallest_neighbor = (unsigned int )-1;
            smallest_neighbor= min(smallest_neighbor, sums[x - 1]);
            smallest_neighbor= min(smallest_neighbor, sums[x - 2]);
            smallest_neighbor= min(smallest_neighbor, sums[x - 3]);
            smallest_neighbor= min(smallest_neighbor, sums[x + 1]);
            smallest_neighbor= min(smallest_neighbor, sums[x + 2]);
            smallest_neighbor= min(smallest_neighbor, sums[x + 3]);
            if (sums[x] > smallest_neighbor * local_compare) {
                ret.push_back(x);
            }
        }
    }

    return ret;
}

vector<unsigned int>
ImageHorizontalLines(Image &image, unsigned int from, unsigned int to)
{

    int width = image.columns();
    int height = image.rows();
    int length = height;
    int block_length = width;
    from = max(0, (int)from);
    to = min((int)block_length, (int)to);

    auto pixels = image.getConstPixels(0, 0, image.columns(), image.rows());
    auto ch = image.channels();
    auto getColor = [pixels, width, ch](unsigned int i, unsigned int j){
        // __typeof__(pixels) ptr = pixels + i * width + j;
        // Quantum q = min(min(ptr->red, ptr->blue), ptr->green);
        unsigned offset = ch * (width * j + i);
        Quantum q = min(min(pixels[offset+0], pixels[offset+2]), pixels[offset+1]);
	return (unsigned int)q;
    };

    return DetectLines(length, from, to, getColor);
}


vector<unsigned int>
ImageVerticalLines(Image &image, unsigned int from, unsigned int to)
{
    int width = image.columns();
    int height = image.rows();
    int length = width;
    int block_length = height;
    from = max(0, (int)from);
    to = min((int)block_length, (int)to);

    auto pixels = image.getConstPixels(0, 0, image.columns(), image.rows());
    auto ch = image.channels();
    auto getColor = [pixels, width, ch](unsigned int i, unsigned int j){
        // __typeof__(pixels) ptr = pixels + j * width + i;
        // Quantum q = min(min(ptr->red, ptr->blue), ptr->green);
	unsigned offset = ch * (width * j + i);
        Quantum q = min(min(pixels[offset+0], pixels[offset+2]), pixels[offset+1]);
        return (unsigned int)q;
    };
    return DetectLines(length, from, to, getColor);
}


unsigned int
LineFeatureCount(const LineFeature & lf)
{
    auto vhLines = lf.get<1>();
    return vhLines.get<1>().size() + vhLines.get<3>().size();
}

ImageVHLines
ImageVHLinesRotate(const ImageVHLines &lines, int rotation)
{
    ImageVHLines ret;
    switch (rotation) {
    case 90:
        {
//            unsigned int width = lines.get<2>();
//            auto preH = lines.get<3>();
//            vector<unsigned int> v(preH.size());
//            auto last = preH.size() - 1;
//            for (int i = last; i >= 0; i --) {
//                v[last - i] = width - preH[i];
//            }
//
//            unsigned int height = lines.get<0>();
//            auto preV = lines.get<1>();
//            vector<unsigned int> h(preV.size());
//            for (int i = preV.size() - 1; i >= 0; i --) {
//                h[i] = preV[i];
//            }
//
//            ret = boost::make_tuple(width, v, height, h);

            unsigned int width = lines.get<2>();
            auto preH = lines.get<3>();
            vector<unsigned int> v(preH.size());
            auto last = preH.size() - 1;
            for (int i = last; i >= 0; i --) {
                v[i] = preH[i];
            }

            unsigned int height = lines.get<0>();
            auto preV = lines.get<1>();
            vector<unsigned int> h(preV.size());
            last = preV.size() - 1;
            for (int i = last; i >= 0; i --) {
                h[last - i] = height - preV[i];
            }
            ret = boost::make_tuple(width, v, height, h);
        }
        break;
    case 180:
        {
            ret = ImageVHLinesRotate(ImageVHLinesRotate(lines, 90), 90);
        }
        break;
    case 270:
        {
            ret = ImageVHLinesRotate(ImageVHLinesRotate(lines, 180), 90);
        }
        break;
    default:
        {
            ret = lines;
        }
    }
    return ret;
}

void
writeAugmentedImage(const Image &image, const ImageVHLines & vhLines, const string file)
{
    Image img(image);
    auto w = vhLines.get<0>();
    auto wLines = vhLines.get<1>();
    auto h = vhLines.get<2>();
    auto hLines = vhLines.get<3>();
    Color black(0, 0, 0);

    BOOST_FOREACH(unsigned int i, wLines) {
        for (int j = 0; j < h; j++) {
            img.pixelColor(i, j, black);
        }
    }

    BOOST_FOREACH(unsigned int j, hLines) {
        for (int i = 0; i < w; i++) {
            img.pixelColor(i, j, black);
        }
    }
    img.write(file);
}

void tImageVHLinesRotate(void)
{
    string fileName = "./images/a.jpg";
    Image image(fileName);
    auto vhLines = ImageLines(image);
    writeAugmentedImage(image, vhLines, "/var/www/yjf/augmented-origin-a.jpg");
    cout << "origin:\n" << ImageVHLinesToString(vhLines) << endl;
    auto rotated0 = ImageVHLinesRotate(vhLines, 90);
    cout << "fast:\n" << ImageVHLinesToString(rotated0) << endl;
    image.rotate(90);
    writeAugmentedImage(image, rotated0, "/var/www/yjf/augmented-fast-rotated-a.jpg");
    image.write("./images/a-90.jpg");
    auto vhLines1 = ImageLines(image);
    writeAugmentedImage(image, vhLines1, "/var/www/yjf/augmented-precise-rotated-a.jpg");
    cout << "precise:\n" << ImageVHLinesToString(vhLines1) << endl;
}


struct IntDefaulted
{
    int v = DIFF_DELTA * DIFF_DELTA;
    static IntDefaulted newOne(int i)
    {
      IntDefaulted x;
      x.v = i;
      return x;
    }
};


tuples::tuple<unsigned int, float, unsigned int>
ImageLinesDiff(vector<unsigned int> &independent, vector<unsigned int> &dependent, unsigned int & threshold, int init = 0)
{
    int delta = DIFF_DELTA;
    map<int, IntDefaulted> d_independent;
    map<int, IntDefaulted> d_dependent;
    for (int i = 0; i < independent.size(); i++) {
        int index = independent[i];
        for (int j = index - delta; j <= index + delta; j++) {
            int d = (j - index) * (j - index);
            d = min(d, d_independent[j].v);
            d_independent[j] = IntDefaulted::newOne(d);
        }
    }

    for (int i = 0; i < dependent.size(); i++) {
        int index = dependent[i];
        for (int j = index - delta; j <= index + delta; j++) {
            int d = (j - index) * (j - index);
            d = min(d, d_dependent[j].v);
            d_dependent[j] = IntDefaulted::newOne(d);
        }
    }

    auto foreachN = xl_bind<int, int>(xl_N(PANNING_LIMIT),
            [](int i) {
                xlist(int) ret = [i](xlist_cb(int) cb) {
                    cb(i);
                    cb(-i);
                };
                return ret;
            });
    unsigned int diff = threshold;
    float k = 1;
    int b = 0;
    unsigned int counter = 0;
    auto getDiffValue = [&counter](unsigned int value = 0) {
      if (value == DIFF_DELTA * DIFF_DELTA && counter == 0) {//允许有一个检测出错
        value /= 100;
        counter ++;
      }
      return value;
    };

    foreachN([getDiffValue, &diff, &threshold, &b, init, &d_dependent,
            &d_independent, &independent, &dependent](int x) {
        unsigned int d = init;
        for (int i = 0; i < independent.size(); i ++) {
            int index = independent[i] + x;
            d += getDiffValue(d_dependent[index].v);
            if (d > diff) {
                break;
            }
        }

        for (int i = 0; i < dependent.size(); i ++) {
            int index = dependent[i] - x;
            d += getDiffValue(d_independent[index].v);
            if (d > diff) {
                break;
            }
        }
//        cout << x << "\t" << diff << "\t" << d << endl;
        if (d > 0 && d < diff) {
            diff = d;
            b = x;
        }
    });

    return boost::make_tuple(diff, k, b);
}

boost::tuple<unsigned int, float, unsigned int, unsigned int>
ImageVHLinesDiff(const ImageVHLines &independent, const ImageVHLines &dependent, unsigned int & threshold)
{
    float k = 1;

    auto v0 = independent.get<1>();
    auto h0 = independent.get<3>();
    auto v1 = dependent.get<1>();
    auto h1 = dependent.get<3>();
    auto ret = ImageLinesDiff(v0, v1, threshold);
    auto t = ret.get<0>();
    auto bv = ret.get<2>();


    ret = ImageLinesDiff(h0, h1, threshold, t);

    t += ret.get<0>();
    auto bh = ret.get<2>();

    return boost::make_tuple(t, k, bv, bh);
}

void tImageLinesDiff(void)
{
    vector<unsigned int> v0 = {
        1, 4, 6, 8, 9, 10, 20
    };
    vector<unsigned int> v1 = {
        21, 24, 26, 28, 29, 30, 40
    };
    unsigned int threshold = UINT_MAX;
    auto t = ImageLinesDiff(v0, v1, threshold);
    cout << t.get<0>() << "\t" << t.get<1>() << "\t" << t.get<2>() << endl;
}


ImageVHLines
ImageLines(Image &image)
{
    int nr_part = NR_SLICE;
    map<unsigned int, unsigned int> vertical_map;
    for (int x = 0; x < nr_part; x++) {
        auto vec = ImageVerticalLines(image,
                x * image.rows() / nr_part, (x + 1) * image.rows() / nr_part);
        for (int i = 0; i < vec.size(); i++) {
            vertical_map[vec[i]] = vec[i];
        }
    }
    vector<unsigned int> vertical;
    for (auto itt = vertical_map.begin(); itt != vertical_map.end(); itt++) {
        vertical.push_back(itt->first);
    }

    map<unsigned int, unsigned int> horizontal_map;
    for (int x = 0; x < nr_part; x++) {
        auto vec = ImageHorizontalLines(image,
                x * image.columns() / nr_part, (x + 1) * image.columns() / nr_part);
        for (int i = 0; i < vec.size(); i++) {
            horizontal_map[vec[i]] = vec[i];
        }
    }

    vector<unsigned int> horizontal;
    for (auto itt = horizontal_map.begin(); itt != horizontal_map.end(); itt++) {
        horizontal.push_back(itt->first);
    }

    return boost::make_tuple(
            (unsigned int)image.columns(),
            vertical,
            (unsigned int)image.rows(),
            horizontal);
}


struct ClassifiedResult {
        unsigned int diff = UINT_MAX;
        float k = 1;
        int bv = 0;
        int bh = 0;
        string templateName = "";
        int templateFeatureCount = 0;
        string name = "";
        int featureCount = 0;
        int rotation = 0;
};

string
ClassifiedResultToJson(const vector<ClassifiedResult> &results)
{
//    cout << results.size() << endl;
    Json::Value root(Json::arrayValue);
    BOOST_FOREACH(const ClassifiedResult & result, results) {
        Json::Value item;
        item["diff"] = result.diff;
        item["k"] = result.k;
        item["bw"] = result.bv;
        item["bh"] = result.bh;
        item["template_name"] = result.templateName;
        item["template_feature_count"] = result.templateFeatureCount;
        item["name"] = result.name;
        item["feature_count"] = result.featureCount;
        item["rotation"] = result.rotation;
        root.append(item);
    }
    string ret = json_encode(root);
    return ret;
}

vector<ClassifiedResult>
ImageClassify(
        const vector<LineFeature> & featureClasses,
        const vector<LineFeature> & lineFeatures)
{
    vector<unsigned int> rotations = {0, 90, 180, 270};
    auto foreachRotations = vector2xl<unsigned int>(rotations);
    auto foreachTemplate = vector2xl<LineFeature>(featureClasses);
    auto foreachCase = xl_product<unsigned int, LineFeature>(
            foreachRotations, foreachTemplate);

    auto foreachLineFeature = vector2xl<LineFeature>(lineFeatures);
    auto foreachClassified = xl_map<LineFeature, ClassifiedResult>(foreachLineFeature,
            [foreachCase](LineFeature lineFeature) {
            ClassifiedResult result;
            {
                result.diff = UINT_MAX;
                result.k = 1;
                result.bv = 0;
                result.bh = 0;
                result.templateName = "";
                result.name = lineFeature.get<0>();
                result.featureCount = LineFeatureCount(lineFeature);
                result.rotation = 0;
            }
            foreachCase([&result, &lineFeature]
                (boost::tuple<unsigned int, LineFeature> t) {
                unsigned int threshold = result.diff;

                auto vhLines = lineFeature.get<1>();
                auto r = t.get<0>();
                auto rotated = ImageVHLinesRotate(vhLines, r);

                auto lineTemplate = t.get<1>();
                auto templateVHLines = lineTemplate.get<1>();

                auto ret = ImageVHLinesDiff(rotated, templateVHLines, threshold);
                //auto oldRet = ImageVHLinesDiff(vhLines, templateVHLines, threshold);
//                cout << "origin: diff=" << ret.get<0>() << "\n" << ImageVHLinesToString(vhLines) << endl;
//                cout << "rotated(" << r << "): diff=" << ret.get<0>() << "\n" << ImageVHLinesToString(rotated) << endl;

                auto diff = ret.get<0>();
                if (diff < threshold) {
                    result.diff = diff;
                    result.k = ret.get<1>();
                    result.bv = ret.get<2>();
                    result.bh = ret.get<3>();
                    result.templateName = lineTemplate.get<0>();
                    result.templateFeatureCount = LineFeatureCount(lineTemplate);
                    result.rotation = r;
                }
            });

            /**
             * 由方差算平方差
             */
            auto t = result.diff;

            unsigned int length = result.templateFeatureCount + result.featureCount;

            t /= length ?: UINT_MAX;
            t += 1;
            t = (int)sqrt((float)t);
            t += 1;

            result.diff = t;
            return result;
        });
    auto classifiedResults = xl2vector<ClassifiedResult>(foreachClassified);
    return classifiedResults;
}

vector<LineFeature>
ImageNames2LineFeatures(const vector<string> &ImageNames)
{
    auto foreachName = vector2xl(ImageNames);
    auto foreachLineFeatures =
        xl_map<string, LineFeature>(foreachName, [](string file) {

      Image image;
        LineFeature lineFeature;
//        cout << now() << "\tbegin\t" << file << endl;
        try {
            image.read(file);
            auto vhLines = ImageLines(image);
             lineFeature = boost::make_tuple(file, vhLines);
        } catch (Exception &e) {
        }
//        cout << now() << "\tend\t" << file << endl;
        return lineFeature;
    });
    auto foreachValidLineFeatures = xl_filter<LineFeature>(foreachLineFeatures,
            [](LineFeature lineFeature){
                auto vnLines = lineFeature.get<1>();
                auto w = vnLines.get<0>();
                auto h = vnLines.get<2>();
                return w > 0 && h > 0;
            });
    auto vec = xl2vector<LineFeature>(foreachValidLineFeatures);
    return vec;
}

void PartialImage_colorSample( PartialImage &pi, vector<vector<float>> &matrix)
{
//    __ENTER__();
    int width = pi.image.columns();
    int height = pi.image.rows();
    int areaX = max(0, (int)(pi.x * width));
    int areaY = max(0, (int)(pi.y * height));
    int areaWidth = min(width, (int)(pi.w * width));
    int areaHeight = min(height, (int)(pi.h * height));
    int areaEndX = areaX + areaWidth;
    int areaEndY = areaY + areaHeight;
    float stepX = max((float)1, (float) areaWidth / SAMPLE_SIZE);
    float stepY = max((float)1, (float) areaHeight / SAMPLE_SIZE);

    auto pixels = pi.image.getConstPixels(0, 0, pi.image.columns(), pi.image.rows());
    auto ch = pi.image.channels();
    auto getColor = [pixels, width, ch](unsigned int i, unsigned int j) {
        // __typeof__(pixels) ptr = pixels + j * width + i;
        // Quantum q = min(min(ptr->red, ptr->blue), ptr->green);
	unsigned offset = ch * (width * j + i);
        Quantum q = min(min(pixels[offset+0], pixels[offset+2]), pixels[offset+1]);
        return (unsigned int)q;
    };

    for (float i = areaX, indexX = 0;
                i < areaEndX && indexX < SAMPLE_SIZE;
                i += stepX, indexX++) {
        for (float j = areaY, indexY = 0;
                j < areaEndY && indexY < SAMPLE_SIZE; j += stepY, indexY++) {

            int gray = 0;
            int counter = 1;
            for (int s = 0; s < stepX; s ++) {
                for (int t = 0; t < stepY; t++) {
                    if (i + s < width && j + t < height) {
                        auto rgb = getColor((unsigned int)(i + s), (unsigned int)(j + t));
                        rgb = (rgb & 0xff);
                        gray += rgb / 16;
                        counter ++;
                    }
                }
            }
            matrix[indexX][indexY] =  (int)(gray / counter);
        }
    }
//    __LEAVE__();
}

void Matrix_printDistribution(Matrix &matrix)
{
    int colorVector[16] = {0};
    BOOST_FOREACH(vector<float> &row, matrix) {
        BOOST_FOREACH(float x, row) {
            int index = max(0, min(15, (int)x));
            colorVector[index] ++;
        }
    }
    cout << &matrix << endl;
    for (int i = 0; i < 16; i ++) {
        cout << i << ":" << colorVector[i] << "\t";
    }
    cout << endl;
}

inline void SampleMatrix_init(vector<vector<float>> &matrix)
{
//    __ENTER__();
    for (int i = 0; i < matrix.size(); i ++) {
        matrix[i].resize(matrix.size());
    }
//    __LEAVE__();
}

vector<vector<float>> SampleMatrix_new(int size = SAMPLE_SIZE)
{
//    __ENTER__();
    vector<vector<float>> ret(size);
    SampleMatrix_init(ret);
//    __LEAVE__();
    return ret;
}

vector<vector<float>> Matrix_sample(vector<vector<float>> &matrix)
{
//    __ENTER__();
    vector<vector<float>> sampled = SampleMatrix_new(matrix.size() / 2);
    if (matrix.size() == 1) {
        return matrix;
    }
    for (int i = 0; i + 1 < matrix.size(); i += 2) {
        for (int j = 0; j + 1 < matrix[i].size() && j + 1 < matrix[i + 1].size(); j += 2) {
            sampled[i / 2][j / 2] =
                (matrix[i][j] + matrix[i][j + 1] + matrix[i + 1][j] + matrix[i + 1][j + 1]) / 4;
        }
    }
//    __LEAVE__();
    return sampled;
}

float Distribution_entropy(vector<float> &distro)
{
//    __ENTER__();
    float sum = 0;
    BOOST_FOREACH(float x, distro) {
        if (x > EPSILON) {
            sum += -1 * x * log(x);
        }
    }
//    __LEAVE__();
    return sum;
}


float Matrix_entropy(vector<vector<float>> &matrix)
{
//    __ENTER__();
    if (!matrix.size()) {
        return 0;
    }
    vector<float> colorCount(nr_color_grade);
    fill_n(colorCount.begin(), nr_color_grade, 0);
    BOOST_FOREACH(vector<float> &row, matrix) {
        BOOST_FOREACH(float i, row) {
            int index = max(0, min((int)i, (int)nr_color_grade - 1));
            colorCount[index]++;
        }
    }

    int totalCount = matrix.size() * matrix.size();
    for (int i = 0 ; i < nr_color_grade; i++) {
        colorCount[i] = colorCount[i] / totalCount;
    }

    float ret = Distribution_entropy(colorCount);
//    __LEAVE__();
    return ret;
}

Matrix Martix_partial(Matrix &matrix, int x, int y, int length)
{
//    __ENTER__();
    Matrix partial = SampleMatrix_new(length);
    for (int i = 0; i < length; i ++) {
        for (int j = 0; j < length; j++) {
            partial[i][j] = matrix[x + i][x + j];
        }
    }
//    __LEAVE__();
    return partial;
}

vector<vector<Matrix>> Matrix_partition(Matrix &matrix, int n = 2)
{
//    __ENTER__();
    vector<vector<Matrix>> ret(n);
    n = max(2, n);
    int matrixRank = matrix.size();
    int subMatrixRank = matrixRank / n;
    for (int i = 0; i < n; i++) {
        vector<Matrix> row(n);
        for (int j = 0; j < n; j++) {
            row[j] = Martix_partial(matrix,
                                    matrixRank * i / n, matrixRank * j / n, subMatrixRank);
        }
        ret[i] = row;
    }
//    __LEAVE__();
    return ret;
}

float Matrix_localEntropy(vector<vector<float>> &matrix, int threshold = 8)
{
//    __ENTER__();
    float ret = 0;
    if (matrix.size() <= threshold) {
        ret = Matrix_entropy(matrix);
    } else {
        vector<vector<Matrix>> partition = Matrix_partition(matrix);
        ret = Matrix_localEntropy(partition[0][0]) +
                    Matrix_localEntropy(partition[0][1]) +
                    Matrix_localEntropy(partition[1][0]) +
                    Matrix_localEntropy(partition[1][1]);
    }
//    __LEAVE__();
    return ret;
}

float Matrix_binaryEntropy(vector<vector<float>> &matrix, int threshold = 8)
{
//    __ENTER__();
    float ret = 0;
    threshold = max(1, threshold);
    if (matrix.size() <= threshold) {
        ret = Matrix_localEntropy(matrix, threshold);
    } else {
        vector<vector<float>> sampled = Matrix_sample(matrix);
        ret = 0.2 * Matrix_localEntropy(matrix, threshold) +
                    0.8 * Matrix_binaryEntropy(sampled, threshold);
    }
//    __LEAVE__();
    return ret;
}

float PartialImage_entropy(PartialImage &pi)
{
//    __ENTER__();
    vector<vector<float>> matrix = SampleMatrix_new();
    PartialImage_colorSample(pi, matrix);
    float ret = Matrix_binaryEntropy(matrix);
//    __LEAVE__();
    return ret;
}

vector<PartialImage> PartialImage_makeVectorFromJson(string str)
{
//    __ENTER__();
    Json::Value root;
    json_decode(str, root);
    vector<PartialImage> ret;
    if (root.type() == Json::ValueType::arrayValue) {
        for (int i = 0; i < root.size(); i++) {
            PartialImage partialImage;
//            cout << "id:" << root[i]["id"].asString() << endl
//                << "x:" << root[i]["x"].asString()
//                << "\ty:" << root[i]["y"].asString()
//                << "\tw:" << root[i]["w"].asString()
//                << "\th:" << root[i]["h"].asString()
//                << endl << endl;
            partialImage.filename = root[i]["filename"].asString();
            partialImage.id = root[i]["id"].asString();
            partialImage.x = root[i]["x"].asFloat();
            partialImage.y = root[i]["y"].asFloat();
            partialImage.w = root[i]["w"].asFloat();
            partialImage.h = root[i]["h"].asFloat();

            partialImage.image.read(partialImage.filename);

            ret.push_back(partialImage);
        }
    }
//    __LEAVE__();
    return ret;
}

string PartialImage_vector2Json(vector<PartialImage> vec)
{
//    __ENTER__();
    Json::Value root(Json::arrayValue);
    BOOST_FOREACH(const PartialImage & pi, vec) {
        Json::Value item;
        item["id"] = pi.id;
        item["filename"] = pi.filename;
        item["x"]             = pi.x;
        item["y"]             = pi.y;
        item["w"]             = pi.w;
        item["h"]             = pi.h;
        item["entropy"] = pi.entropy;
        root.append(item);
    }
    string ret = json_encode(root);
//    __LEAVE__();
    return ret;
}

string YImageEntropy(string str)
{
//    __ENTER__();
    auto partialImageVector = PartialImage_makeVectorFromJson(str);
    BOOST_FOREACH(PartialImage & pi, partialImageVector) {
        pi.entropy = PartialImage_entropy(pi);
    }
//    __LEAVE__();
    return PartialImage_vector2Json(partialImageVector);
}

string YImageClassify(string templateStr, string str)
{
    auto templateVec = json2vector(templateStr);
    auto vec = json2vector(str);
    auto templateFeatures = ImageNames2LineFeatures(templateVec);
    auto features = ImageNames2LineFeatures(vec);
    auto classifiedRet = ImageClassify(templateFeatures, features);
    auto json = ClassifiedResultToJson(classifiedRet);
    return json;
}

string
ImageVHLinesToString(const ImageVHLines &vhLines)
{
    string ret;
    auto w = vhLines.get<0>();
    auto vLines = vhLines.get<1>();
    auto h = vhLines.get<2>();
    auto hLines = vhLines.get<3>();
    char data[256];
    sprintf(data, "w(%d): ", w);
    ret += data;
    for_each(vLines.begin(), vLines.end(), [&ret, &data](unsigned int x){
            sprintf(data, "%d ", x);
        ret += data;
    });
    ret += "\n";
    sprintf(data, "h(%d): ", h);
    ret += data;
    for_each(hLines.begin(), hLines.end(), [&ret, &data](unsigned int x){
            sprintf(data, "%d ", x);
        ret += data;
    });
    ret += "\n";
    return ret;
}

string
LineFeaturesToString(const vector<LineFeature> &lineFeatures)
{
    string ret;
    auto foreachLineFeature = vector2xl(lineFeatures);
    foreachLineFeature([&ret](LineFeature lineFeature){
            auto name = lineFeature.get<0>();
            auto vhLines = lineFeature.get<1>();
            auto w = vhLines.get<0>();
            auto vLines = vhLines.get<1>();
            auto h = vhLines.get<2>();
            auto hLines = vhLines.get<3>();
            ret += name;
            ret += "\n";
            ret += ImageVHLinesToString(vhLines);
    });
    return ret;
}
