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
#define BLACK_THRESHOLD (0x80)
#define SHORT_LINE_LEN_MIN (8)
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

static char *image_path;
static char *dir_path;

int main(int argc,char **argv)
{
	unsigned int x, y, len, w, h;
	int v_offset = 0, h_offset = 0;

	Color red(255, 255, 255);
	string direction;
	function<void(unsigned int, unsigned int, unsigned int)> set_red;

	if (argc < 3) {
		usage("invalid argument");
	}
	
	if (!is_dir_exist(argv[1])) {
		usage("new image directory not exist");
	}

	if (access(argv[1], W_OK)) {
		usage("new image directory no permittion to write");
	}

	dir_path = argv[1];

	if (access(argv[2], R_OK)) {
		usage("image not exists or no reading permittion");
	}

	image_path = argv[2];

	for (int i = 3; i < argc - 1; i++) {
		if (!strcmp(argv[i], "--vertical")) {
			if (sscanf(argv[i + 1], "%d", &v_offset) != 1) {
				usage("argument of --vertical should be an integer");
			}
			i ++;
		} else if (!strcmp(argv[i], "--horizontal")) {
			if (sscanf(argv[i + 1], "%d", &h_offset) != 1) {
				usage("argument of --horizontal should be an integer");
			}
			i ++;
		} else {
			cout << argv[i] << endl;
				usage("argument invalid");
		}
	}

//	cout << argc << "\t" << v_offset << "\t" << h_offset << endl;

	InitializeMagick(argv[0]);
	string new_file_path(dir_path);
	new_file_path.append("/");
	new_file_path.append(basename(image_path));
	Image image(image_path);
	auto new_image = deskew(image, v_offset, h_offset);
	new_image.write(new_file_path);
	return 0;
}
