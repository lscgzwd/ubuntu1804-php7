#include <Magick++.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>
#include <string.h>
#include "image.h"

using namespace std;
using namespace Magick;

static void usage(const char *msg)
{
	cerr << msg << endl << endl;
	cerr << "usage: deskew-image <new-path-dir> <image-path> "
		" --horizontal-offset <offset>"
		" --vertical-offset <offset>"	<< endl;
	exit(-1);
}

static bool is_dir_exist(const char *dir)
{
	return dir && opendir(dir) ? true : false;
}

static char *image_path = NULL;
static char *dir_path = NULL;

static char *resize = NULL;
static vector<char *> images;

int main(int argc,char **argv)
{
	images = vector<char *>(10);
	images.clear();

	for (int i = 1; i < argc; i ++) {
		if (!strcmp(argv[i], "--path") && i + 1 < argc) {
			dir_path = argv[i + 1];
			i ++;
		} else if (!strcmp(argv[i], "--resize") && i + 1 < argc) {
			resize = argv[i + 1];
			i ++;
		} else {
			images.push_back(argv[i]);
		}
	}

	if (!is_dir_exist(dir_path)) {
		usage("new image directory not exist");
	}

	if (access(dir_path, W_OK)) {
		usage("new image directory no permittion to write");
	}

	InitializeMagick(argv[0]);
	for (int i = 0; i < images.size(); i++) {
		auto image_path = images[i];
		string new_file_path(dir_path);
		new_file_path.append("/");
		new_file_path.append(basename(image_path));
		Image image;
		try {
			image.read(image_path);
			if (resize) {
				image.resize(resize);
				/**
				 * there is a unknown bug in Image::resize that
				 * we couldn't use resized image immediately.
				 * now we have to write it to a tmp file then reload it,
				 * IT'S SUCKS!
				 */
				auto tmp = tmp_image_file();
				image.write(tmp);
				image = Image(tmp);
				unlink(tmp.c_str());
			}
			erase_black_margin(image);
			int v_offset = 0, h_offset = 0;
			auto vlines = extract_vlines(image);
			v_offset = simple_line_offset(*vlines, image.columns(), image.rows());
			auto hlines = extract_hlines(image);
			h_offset = simple_line_offset(*hlines, image.rows(), image.columns());
			deskew(image, v_offset, h_offset);
			image.write(new_file_path);
		} catch (Exception &e) {
			cerr << e.what() << endl;
		}
	}

	return 0;
}
