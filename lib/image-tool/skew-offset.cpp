#include <string.h>
#include <iostream>
#include <algorithm>
#include "image.h"
#define MARGIN_RATIO (0.03)
using namespace std;


int main(int argc,char **argv)
{
	unsigned int x, y, len;
	unsigned int first_dim_len, second_dim_len;
	string direction;

	cin >> first_dim_len >> second_dim_len;
	vector<struct simple_line> lines(1000);
	lines.clear();

	unsigned int margin = (unsigned int) ((float)first_dim_len * MARGIN_RATIO);

	while (cin >> x >> y >> len) {
		struct simple_line line;
		if (x >= margin && x + margin < first_dim_len) {
			line.first_dim = x;
			line.second_dim = y;
			line.len = len;
			lines.push_back(line);
		}
	}

	cout << simple_line_offset(lines, first_dim_len, second_dim_len) << endl;
	return 0;
}
