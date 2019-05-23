#ifndef IMAGE_H
#include <Magick++.h>
#include <unordered_map>
#include <iostream>
#include <memory>
#include "utils.h"

#define YIMAGE_DEPRECATED

struct Rectangle {
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int w = 1;
	unsigned int h = 1;

	Rectangle() {}
	Rectangle(unsigned int x, unsigned int y, unsigned int w, unsigned int h):
		x(x), y(y), w(w), h(h) {}
};

std::tuple<Rectangle, unsigned int>
biggest_black_rect(Magick::Image &image,
		unsigned int threshold = 128,
		unsigned int w_limit = 23, unsigned int h_limit = 16);


struct simple_line {
	unsigned int first_dim;
	unsigned int second_dim;
	unsigned int len;

	simple_line clone(const simple_line&);
	operator std::string();

	std::vector<simple_line> trim(std::vector<simple_line> &lines);
};
std::vector<simple_line>* sline_thin(std::vector<simple_line> &lines);
struct cmp_range
{
    bool operator () (const std::tuple<int, int> &a, const std::tuple<int, int> &b) 
    {   
        return std::get<0>(a) < std::get<0>(b) && std::get<1>(a) < std::get<1>(b);
    }   
};

/*
 * 等势图
 */
struct PotentialMap
{
	typedef std::map<std::tuple<int, int>, Integer<100000>, cmp_range> Range2Line;

	std::unordered_map<int, Range2Line> assoc_neighbor;
	unsigned int first_dim_len;

	PotentialMap() {}
	PotentialMap(std::vector<simple_line> &lines, unsigned int first_dim_len);
	unsigned int get(int x, int y);

	void init_data(std::vector<simple_line> &lines, unsigned int first_dim_len);
	unsigned long lines_diff(
			std::vector<simple_line> &lines,
			unsigned long init,
			std::function<void(unsigned long)> hook,
			std::function<void(int&, int&)> transform);

};

#ifndef PMAP_DELTA
#define PMAP_DELTA 100
#endif
typedef Integer<PMAP_DELTA> IntDefaulted;

YIMAGE_DEPRECATED struct PotentialBitMap
{
	std::unordered_map<int, IntDefaulted> pmap;

	PotentialBitMap(std::vector<simple_line> &lines);
	unsigned int get(int x, int y);
};

extern std::string tmp_image_file(void);

extern std::shared_ptr<std::vector<simple_line>>
extract_vlines(Magick::Image &image, bool trim_short_line = false);

extern std::shared_ptr<std::vector<simple_line>>
extract_hlines(Magick::Image &image, bool trim_short_line = false);

void erase_black_margin(Magick::Image &image);

extern unsigned long calc_weight(std::vector<simple_line> &lines,
		unsigned int first_dim_len, unsigned int second_dim_len, int offset);

extern int simple_line_offset(std::vector<simple_line> &lines,
		unsigned int first_dim_len, unsigned int second_dim_len);

extern Magick::Image deskew(Magick::Image &img, int v_offset, int h_offset);

extern std::vector<simple_line>
merge_simple_lines(std::vector<simple_line> &lines,
		unsigned int first_dim_len, unsigned int second_dim_len, int offset);

#define SIMPLE_LINE_SAMPLE_STEP (20) 
extern std::unordered_map<int, IntDefaulted>
potential_map(std::vector<simple_line> &lines, int second_dim_len = 10000);


/**
  * 简单线段集合的仿射距离
  */
struct simple_lines_affine_distance
{
	/**
	  * 平移
	  */ 
	int t; 

	/**
	  * 缩放
	  */
	float s; 

	/**
	  * 距离
	  */
	unsigned long diff;
};

struct SLineImage {
	std::shared_ptr<std::vector<simple_line>> slines;
	unsigned int first_dim_len;
	unsigned int second_dim_len;
	std::vector<unsigned int> first_dim_peaks;
	std::map<unsigned int, unsigned int> first_dim_peak_weight;
	Nearest first_dim_peak_neighbor;
	PotentialMap pmap;

	SLineImage() {
		slines = std::shared_ptr<std::vector<simple_line>>(new std::vector<simple_line>);
	}
	SLineImage(std::shared_ptr<std::vector<simple_line>> &slines,
			unsigned int first_dim_len, unsigned int second_dim_len) :
		slines(slines),
		first_dim_len(first_dim_len),
		second_dim_len(second_dim_len),
		pmap(*slines, first_dim_len) {
		init_data();
	}

	void init_data();

	simple_lines_affine_distance diff(
		SLineImage &target,
        unsigned int t_limit,
        unsigned int s_limit,
        unsigned long init);
};

extern simple_lines_affine_distance
simple_lines_affine_diff(
		std::vector<simple_line> &to,
		std::vector<simple_line> &from,
		unsigned int from_len,
		unsigned int t_limit = 100,
		unsigned int s_limit = 0,
		unsigned long init = 0);

struct ImageLineAffineDistance
{
	std::string templateFileName;
	std::string filename;
	int tx;
	int ty;
	int rotation = 0;
	float sx;
	float sy;
	float diff;

	operator std::string();
};

struct ImageLine
{
	std::shared_ptr<std::vector<simple_line>> horizontal_lines;
	std::shared_ptr<std::vector<simple_line>> vertical_lines;
	unsigned int width;
	unsigned int height;

	std::shared_ptr<SLineImage> horizontal;
	std::shared_ptr<SLineImage> vertical;

	ImageLine() {
		horizontal = std::shared_ptr<SLineImage>(new SLineImage());
		vertical = std::shared_ptr<SLineImage>(new SLineImage());
	}
	ImageLine(Magick::Image&);
	operator std::string();
	ImageLineAffineDistance distance(
			ImageLine &target, unsigned int t_limit = 100,
			unsigned int s_limit = 100);
	unsigned int roundup_len();
};

struct RotableImage
{
	std::string filename;
	std::map<int, std::shared_ptr<ImageLine>> image_lines;

	RotableImage() {}
	RotableImage(std::string filename);
};

struct Configure
{
	bool full_rotated = false;
	unsigned int t_limit = 100;
	unsigned int s_limit = 300;
};


/**
  * 候选图片，对应多张答题卡
  */
struct CandidateImages
{
	Configure config;
	std::vector<RotableImage> candidates;

	CandidateImages(std::vector<std::string> filenames);
	CandidateImages(const std::initializer_list<std::string> & filenames);
	void for_each(std::function<void(const std::string&, int rotation, ImageLine &)> cb);
	
	ImageLineAffineDistance nearest(Magick::Image &image,
			unsigned int t_limit = 100, unsigned int s_limit = 100);

	ImageLineAffineDistance nearest(std::string filename,
			unsigned int t_limit = 100, unsigned int s_limit = 100);

	void drawTemplateLine(Magick::Image &image, ImageLineAffineDistance& d);
};

std::string classify_by_line(std::string templates, std::string images);
#endif/*IMAGE_H*/
