#include<Magick++.h>
#include<string.h>
#include<iostream>
#include<algorithm>
#include "image.h"
using namespace std;
using namespace Magick;


static void print_line(struct simple_line l)
{
	printf("%d %d %d\n", l.first_dim, l.second_dim, l.len);
}


int main(int argc,char **argv)
{
	InitializeMagick(argv[0]);
	if (argc < 3) {
		cerr << "usage: extract-lines --[vertical|horizontal] "
			"<image-path> <image-path>..." << endl;
		exit(-1);
	}
	Image image(argv[2]);
	if (!strcmp(argv[1], "--vertical")) {
		cout << image.columns() << " " << image.rows() << endl;
		auto lines = extract_vlines(image, true);
		auto l = sline_thin(*lines);
//		for_each(lines->begin(), lines->end(), print_line);
		for_each(l->begin(), l->end(), print_line);
	} else if (!strcmp(argv[1], "--horizontal")) {
		cout << image.rows() << " " << image.columns() << endl;
		auto lines = extract_hlines(image, true);
		auto l = sline_thin(*lines);
//		for_each(lines->begin(), lines->end(), print_line);
		for_each(l->begin(), l->end(), print_line);
	} else {
		cerr << "usage: extract-lines --[vertical|horizontal] "
			"<image-path> <image-path>..." << endl;
		exit(-1);
	}
	return 0;
}
