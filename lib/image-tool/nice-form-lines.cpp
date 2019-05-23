#include<Magick++.h>
#include<string.h>
#include<iostream>
#include<algorithm>
#define BLACK_THRESHOLD (0x80)
#define SHORT_LINE_LEN_MIN (8)
using namespace std;
using namespace Magick;

struct simple_line {
	unsigned int first_dim;
	unsigned int second_dim;
	unsigned int len;
};

int main(int argc,char **argv)
{
	unsigned int x, y, len, w, h;
	string direction;
	vector<struct simple_line*> lines(1000);
	lines.clear();

	function<void(struct simple_line*,unsigned int, unsigned int)> set_simple_line;

	cin >> direction >> w >> h;
	if (!direction.compare("horizontal")) {
		cout << h << "\t" << w << endl;
		set_simple_line = []
			(struct simple_line *line, unsigned int i, unsigned int j) {
				line->first_dim = j;
				line->second_dim = i;
		};
	} else if (!direction.compare("vertical")) {
		cout << w << "\t" << h << endl;
		set_simple_line = []
			(struct simple_line *line, unsigned int i, unsigned int j) {
				line->first_dim = i;
				line->second_dim = j;
		};
	} else {
		cerr << "usage: invalid direction" << endl;
		exit(-1);
	}
	while (cin >> x >> y >> len) {
		auto *line = new struct simple_line; 
		set_simple_line(line, x, y);
		line->len = len;
		lines.push_back(line);
	}
	for_each(lines.begin(), lines.end(), [](struct simple_line *line){
			cout << line->first_dim << "\t" << line->second_dim
				<< "\t" << line->len << endl;
	});
	return 0;
}
