#include <Magick++.h>
#include <iostream>
#include <string.h>
#include "image.h"
#include "utils.h"
#include <time.h>

using namespace std;
using namespace Magick;

void test_potential_map(void);
void test_potential_map_lines_diff(void);
void test_simple_lines_affine_diff(void);
void test_ImageLine_distance(void);
void test_ImageLine_distance_hard(void);
void test_PotentialMap(void);
void test_PotentialMap_deep(void);
void test_image_PotentialMap_deep(void);

void diff_PotentialMap(vector<simple_line> &lines,
		int second_dim_len, int first_dim_len);

void diff_ImageLine_PotentialMap(ImageLine &il);

void test_CandidateImages_pick(void);
void test_classify_by_line(void);
void test_fn_continuous_number(void);

void test_data_peak(void);
void test_Nearest(void);

void t_biggest_black_rect(void);

int main(int argc,char **argv)
{
	InitializeMagick(argv[0]);
//	test_potential_map();
//	test_potential_map_lines_diff();
//	test_simple_lines_affine_diff();
//	test_ImageLine_distance();
//	test_ImageLine_distance_hard();
//	test_PotentialMap();
//	test_PotentialMap_deep();
//	test_image_PotentialMap_deep();
	test_CandidateImages_pick();
//	test_classify_by_line();
//	test_fn_continuous_number();
//	test_data_peak();
//	test_Nearest();
//	t_biggest_black_rect();
	return 0;
}

void t_biggest_black_rect(void)
{
//	auto file = "/home/yjf/tmp/0/fillable-objective-question.5.C.jpg";
	auto file = "/home/yjf/tmp/0/fillable-objective-question.12.D.jpg";
	Image image(file);
	Blob blob;
	image.write(&blob);
	Blob b;
	b.base64(blob.base64());
	Image img(b);
	Rectangle rect;
	unsigned count;
	tie(rect, count) = biggest_black_rect(img);
	cout << "c=" << count << ", file=" << file << endl <<
		"x=" << rect.x <<
		", y=" << rect.y <<
		", w=" << rect.w <<
		", h=" << rect.h << endl;
}

void test_Nearest(void)
{
	auto vec = vector<int>{0, 5, 20, 100};
	auto neighbor = Nearest(vec);
	cout << neighbor.diff(-3) << endl;
	cout << neighbor.distance(-3) << endl;
	cout << neighbor.diff(3) << endl;
	cout << neighbor.distance(3) << endl;
	cout << neighbor.diff(5) << endl;
	cout << neighbor.distance(5) << endl;
	cout << neighbor.diff(10) << endl;
	cout << neighbor.distance(10) << endl;
	cout << neighbor.diff(50) << endl;
	cout << neighbor.distance(50) << endl;
	cout << neighbor.diff(120) << endl;
	cout << neighbor.distance(120) << endl;
}

void test_data_peak(void)
{
	auto data = vector<unsigned int>{ 0, 10, 2, 3, 4, 0, 0, 6, 2, 9, 3,0 };
	vector<unsigned int> peaks;
	map<unsigned int, unsigned int> peak_weight;
	get_data_peaks(data, peaks, peak_weight, 4);
	for (auto itt = data.begin(); itt != data.end(); itt++) {
		cout << *itt << "\t";
	}
	cout << endl;
	for (auto itt = peaks.begin(); itt != peaks.end(); itt++) {
		cout << *itt << "\t" << peak_weight[*itt] << endl;
	}
}

void test_fn_continuous_number(void)
{
	auto cn = ContinuousNumber();
	cout << cn(0) << endl;
	cout << cn(1) << endl;
	cout << cn(2) << endl;
	cout << cn(4) << endl;
	cout << cn(5) << endl;

	auto foreach = [](function<void(int)> cb){
		cb(0);
		cb(1);
		cb(2);
		cb(4);
		cb(5);
	};
	auto vec = ContinuousNumber::islands(foreach);
	for (int i = 0; i < vec.size(); i++) {
		cout << i << ": ";
		for (int j = 0; j < vec[i].size(); j++) {
			cout << vec[i][j] << " ";
		}
		cout << endl;
	}
}

void test_image_PotentialMap_deep(void)
{
	Image img("./b.jpg");
	ImageLine il(img);
	diff_ImageLine_PotentialMap(il);
	
}

void test_PotentialMap_deep(void)
{
	simple_line sl0 = {
		.first_dim = 100,
		.second_dim = 100,
		.len = 5,
	};

	simple_line sl = {
		.first_dim = 150,
		.second_dim = 100,
		.len = 10,
	};


	simple_line sl1 = {
		.first_dim = 800,
		.second_dim = 100,
		.len = 5,
	};

	ImageLine il;
	il.width = 1414;
	il.height = 1000;
	auto hlines = shared_ptr<vector<simple_line>>(new vector<simple_line>{ sl0, sl });
	il.horizontal = shared_ptr<SLineImage>(new SLineImage(hlines, il.height, il.width));
	auto vlines = shared_ptr<vector<simple_line>>(new vector<simple_line>{ sl1 });
	il.horizontal = shared_ptr<SLineImage>(new SLineImage(vlines, il.width, il.height));

//	Image img("./b.jpg");
//	ImageLine il(img);
	diff_ImageLine_PotentialMap(il);
	
}

void diff_PotentialMap(vector<simple_line> &lines,
		int second_dim_len, int first_dim_len)
{
	PotentialMap pm(lines, first_dim_len);
	PotentialBitMap pbm(lines);
	cout << second_dim_len << " x " << first_dim_len << endl;
	for (int i = 0; i < second_dim_len; i++) {
		for (int j = 0; j < first_dim_len; j ++) {
			auto pmv = pm.get(i, j);
			auto pbmv = pbm.get(i, j);
			if (pm.get(i, j) != pbm.get(i, j)) {
				cout << "(" << i << ", " << j << "): pm=" << pmv << ", pbm=" << pbmv << endl;
			}
		}
	}
}

void diff_ImageLine_PotentialMap(ImageLine &il)
{
	cout << "horizontal-line" << endl;
	diff_PotentialMap(*il.horizontal->slines, il.width, il.height);
	cout << "vertical-line" << endl;
	diff_PotentialMap(*il.vertical->slines, il.height, il.width);
}

void test_PotentialMap(void)
{
	simple_line sl0 = {
		.first_dim = 100,
		.second_dim = 100,
		.len = 1,
	};

	simple_line sl1 = {
		.first_dim = 150,
		.second_dim = 100,
		.len = 1,
	};

	simple_line sl2 = {
		.first_dim = 150,
		.second_dim = 500,
		.len = 1,
	};
	auto lines = vector<simple_line>{ sl0, sl1, sl2 };
	PotentialMap pm(lines, 300);
	cout << pm.get(150, 90) << endl;
	cout << pm.get(150, 140) << endl;
	cout << pm.get(150, 180) << endl;
	cout << pm.get(550, 150) << endl;
	cout << pm.get(550, 151) << endl;
	cout << pm.get(900, 90) << endl;
	cout << pm.get(1200, 90) << endl;
}

void test_classify_by_line(void)
{
	string images = "[\"./a.jpg\", \"./b.jpg\"]";
	cout << classify_by_line(images, images) << endl;
}

void test_CandidateImages_pick(void)
{
//	CandidateImages imgs{"http://s3.cn-north-1.amazonaws.com.cn/pyds/uploads/answer-cards/20170427/e90915f9-9c9e-44d7-a5e4-6900f72ec658.png"};
//	Image image("http://s3.cn-north-1.amazonaws.com.cn/pyds/uploads/raw-user-answers/2017-04-27/deskewed/b5701eed-5ab5-4caf-a63c-1c5cbb5b57b6.jpg");
//	CandidateImages imgs{"http://s3.cn-north-1.amazonaws.com.cn/pyds/uploads/answer-cards/20170426/6836a99c-06ad-42ce-b76e-5999ce6227b2.png", "http://s3.cn-north-1.amazonaws.com.cn/pyds/uploads/answer-cards/20170426/2dfc75cf-8266-40c1-8835-dd44ccbedb65.png"};
//	Image image("http://s3.cn-north-1.amazonaws.com.cn/pyds/uploads/raw-user-answers/2017-04-26/deskewed/eadb05ef-053d-4806-84c0-a5aaf8ea09ca.jpg");
	cout << "start" << endl;
{
 time_t tt = time(NULL);//这句返回的只是一个时间cuo
 tm* t= localtime(&tt);
 printf("%d-%02d-%02d %02d:%02d:%02d\n", 
  t->tm_year + 1900,
  t->tm_mon + 1,
  t->tm_mday,
  t->tm_hour,
  t->tm_min,
  t->tm_sec);
}
	CandidateImages imgs{"http://s3.cn-north-1.amazonaws.com.cn/pyds/uploads/answer-cards/20170309/36cc66a2-4f49-4d9f-9c22-164cde8f2824.png", "http://s3.cn-north-1.amazonaws.com.cn/pyds/uploads/answer-cards/20170309/db5b9fcf-fa17-4b81-9301-09511421e5d9.png", "http://s3.cn-north-1.amazonaws.com.cn/pyds/uploads/answer-cards/20170309/8dad0261-5dab-4830-b8a9-be9b8cad026e.png", "http://s3.cn-north-1.amazonaws.com.cn/pyds/uploads/answer-cards/20170309/31b5b8d1-af60-461f-a5b5-12912babf38f.png"};
	cout << "after-init-candidate-images" << endl;
{
 time_t tt = time(NULL);//这句返回的只是一个时间cuo
 tm* t= localtime(&tt);
 printf("%d-%02d-%02d %02d:%02d:%02d\n", 
  t->tm_year + 1900,
  t->tm_mon + 1,
  t->tm_mday,
  t->tm_hour,
  t->tm_min,
  t->tm_sec);
}
	Image image("http://s3.cn-north-1.amazonaws.com.cn/pyds/uploads/raw-user-answers/2017-03-09/deskewed/314cfee0-978f-4914-9ea5-7f2829ccfcfa.jpg");
//	Image image("http://s3.cn-north-1.amazonaws.com.cn/pyds/uploads/raw-user-answers/2017-03-09/deskewed/f7837712-46d3-4b14-ae24-f25a767ac252.jpg");


	Image origin(image); 
	cout << "good" << endl;

{
 time_t tt = time(NULL);//这句返回的只是一个时间cuo
 tm* t= localtime(&tt);
 printf("%d-%02d-%02d %02d:%02d:%02d\n", 
  t->tm_year + 1900,
  t->tm_mon + 1,
  t->tm_mday,
  t->tm_hour,
  t->tm_min,
  t->tm_sec);
}

	auto d = imgs.nearest(origin);
	cout << "nice" << endl;
{
 time_t tt = time(NULL);//这句返回的只是一个时间cuo
 tm* t= localtime(&tt);
 printf("%d-%02d-%02d %02d:%02d:%02d\n", 
  t->tm_year + 1900,
  t->tm_mon + 1,
  t->tm_mday,
  t->tm_hour,
  t->tm_min,
  t->tm_sec);
}
	cout << (string)d << endl;

	imgs.drawTemplateLine(image, d);

//	auto rotation = 360 - d.rotation;
//	image.rotate(rotation);
//
//	d = imgs.nearest(image);
//
////	imgs.drawTemplateLine(image, d);
//	cout << (string)d << endl;

//	CandidateImages imgs{ "./a.jpg", "./b.jpg" };
////	CandidateImages imgs{ "./a.jpg" };
//	Image image("./b.jpg");
//	image.rotate(90);
//	auto d = imgs.nearest(image);
//	cout << (string)d << endl;
//
//	Image img("./a.jpg");
//	img.rotate(180);
//	auto d1 = imgs.nearest(img);
//	cout << (string)d1 << endl;
}


void test_ImageLine_distance_hard(void)
{
	Image img("./a.jpg");
	ImageLine il(img);

	img.rotate(180);
	ImageLine origin_rotated(img);

	img.rotate(180);
	img.rotate(180);
	ImageLine origin_rotated1(img);

	cout << (string)origin_rotated.distance(origin_rotated1) << endl;
//	cout << (string)il.distance(il) << endl;
}

void test_ImageLine_distance(void)
{

	simple_line sl0 = {
		.first_dim = 100,
		.second_dim = 100,
		.len = 1,
	};

	simple_line sl1 = {
		.first_dim = 800,
		.second_dim = 100,
		.len = 1,
	};

	ImageLine il;
	il.width = 1414;
	il.height = 1000;
	auto hlines = shared_ptr<vector<simple_line>>(new vector<simple_line>{ sl0 });
	il.horizontal = make_shared<SLineImage>(hlines, il.height, il.width);
	auto vlines = shared_ptr<vector<simple_line>>(new vector<simple_line>{ sl1 });
	il.vertical = make_shared<SLineImage>(vlines, il.width, il.height);

	cout << "identity" << endl;
	cout << (string)il.distance(il) << endl;

	cout << (string)il << endl;
	ImageLine empty;
	cout << "empty" << endl;
	cout << (string)il.distance(empty) << endl;

}


void test_SLineImage_diff(void)
{
	simple_line sl = {
		.first_dim = 100,
		.second_dim = 100,
		.len = 100,
	};

	simple_line sl0 = {
		.first_dim = 500,
		.second_dim = 100,
		.len = 100,
	};

	simple_line sl1 = {
		.first_dim = 100,
		.second_dim = 100,
		.len = 100,
	};

	simple_line sl2 = {
		.first_dim = 480,
		.second_dim = 100,
		.len = 100,
	};

	vector<simple_line> vec = vector<simple_line>{ sl, sl0 };
	vector<simple_line> vec0 = vector<simple_line>{ sl1, sl2 };
	auto ret = simple_lines_affine_diff(vec, vec0, 1000, 100, 100);
	cout << ret.s << "\t" << ret.t << "\t" << ret.diff << endl;
}


void test_simple_lines_affine_diff(void)
{
	simple_line sl = {
		.first_dim = 100,
		.second_dim = 100,
		.len = 100,
	};

	simple_line sl0 = {
		.first_dim = 500,
		.second_dim = 100,
		.len = 100,
	};

	simple_line sl1 = {
		.first_dim = 100,
		.second_dim = 100,
		.len = 100,
	};

	simple_line sl2 = {
		.first_dim = 480,
		.second_dim = 100,
		.len = 100,
	};

	vector<simple_line> vec = vector<simple_line>{ sl, sl0 };
	vector<simple_line> vec0 = vector<simple_line>{ sl1, sl2 };
	auto ret = simple_lines_affine_diff(vec, vec0, 1000, 100, 100);
	cout << ret.s << "\t" << ret.t << "\t" << ret.diff << endl;
}

//void test_potential_map_lines_diff(void)
//{
//	simple_line sl = {
//		.first_dim = 100,
//		.second_dim = 100,
//		.len = 1,
//	};
//
//	simple_line sl0 = {
//		.first_dim = 105,
//		.second_dim = 100,
//		.len = 1,
//	};
//
//	simple_line sl1 = {
//		.first_dim = 120,
//		.second_dim = 100,
//		.len = 1,
//	};
//
//	auto vec = vector<simple_line>{ sl, sl0 };
//	auto pmap = potential_map(vec);
//	auto diff = potential_map_lines_diff(pmap, vec, 0);
//	cout << diff << endl;
//	auto vec0 = vector<simple_line>{ sl, sl1 };
//	auto diff0 = potential_map_lines_diff(pmap, vec0, 0);
//	cout << diff0 << endl;
//}

void test_potential_map(void)
{
	simple_line sl = {
		.first_dim = 100,
		.second_dim = 100,
		.len = 1,
	};

	simple_line sl0 = {
		.first_dim = 105,
		.second_dim = 100,
		.len = 1,
	};
	auto vec = vector<simple_line>{ sl, sl0 };
	auto pmap = potential_map(vec);
	for (auto itt = pmap.begin(); itt != pmap.end(); itt ++) {
		int i = itt->first / 10000;
		int j = itt->first % 10000;
		cout << "(" << i << "," << j << ")" << "\t" << itt->second << endl;
	}
}

