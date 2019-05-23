#include<Magick++.h>
#include<string.h>
#include<iostream>
#include<algorithm>
#define BLACK_THRESHOLD (0x80)
#define SHORT_LINE_LEN_MIN (8)
using namespace std;
using namespace Magick;


int main(int argc,char **argv)
{
	unsigned int x, y, len, w, h;
	Color red(65535, 65535, 0);
	string direction;
	string file;
	function<void(unsigned int, unsigned int, unsigned int)> set_red;

	if (argc < 2) {
		cerr << "usage: show-lines <image-path>" << endl;
		exit(-1);
	}

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--horizontal")) {
			direction = "horizontal";
		} else if (!strcmp(argv[i], "--vertical")) {
			direction = "vertical";
		} else {
			file = argv[i];
		}
	}
//	cout << direction << endl;
//	cout << file << endl;
//	exit(-1);

	InitializeMagick(argv[0]);
	Image image(file);
	unsigned int width = image.columns();
	unsigned int height = image.rows();
	if (!direction.compare("horizontal")) {
		cin >> h >> w; 
		set_red = [&image, &red, width, height]
			(unsigned int j, unsigned int i, unsigned int len) {
			if (j >= height) {
				return;
			}
//			if (len < 100) {
//				return;
//			}
			for (; len && i < width; i++, len--) {
				image.pixelColor(i, j, red);
			}
		};
	} else if (!direction.compare("vertical")) {
		cin >> w >> h; 
		set_red = [&image, &red, width, height]
			(unsigned int i, unsigned int j, unsigned int len) {
			if (i >= width) {
				return;
			}
//			if (len < 100) {
//				return;
//			}
			for (; len && j < height; j++, len--) {
				image.pixelColor(i, j, red);
			}
		};
	} else {
		cerr << "usage: invalid direction" << endl;
		exit(-1);
	}
	while (cin >> x >> y >> len) {
		set_red(x, y, len);
	}
	image.write("/home/yjf/tmp.jpg");
	return 0;
}
