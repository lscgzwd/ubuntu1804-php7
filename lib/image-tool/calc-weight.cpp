#include<string.h>
#include<iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <climits>
#include <cmath>   // <-- include cmath here
#define BLACK_THRESHOLD (0x80)
#define SHORT_LINE_LEN_MIN (8)
using namespace std;

int main(int argc,char **argv)
{
	unsigned int x, y, len, w, h;
	string direction;

	cin >> w >> h;
	vector<int> weight(w);
	while (cin >> x >> y >> len) {
		weight[x] += len;
	}
	int ret = 0;
	for (int i = 0; i < weight.size(); i++) {
		ret += weight[i] * weight[i];
	}
	cout << ret << endl;
	return 0;
}
