#include "utils.h"
#include <algorithm>
#include <vector>
#include <map>
#include <climits>
#include <cmath>   // <-- include cmath here
using namespace std;

unsigned int
ContinuousNumber::operator () (int x)
{
	if (last != INT_MIN && last + 1 != x) {
		continuous_number ++;
	}
	last = x;
	return continuous_number;
}

vector<vector<int>>
ContinuousNumber::islands(function<void(function<void(int)>)> foreach)
{
	vector<vector<int>> ret(10);
	ret.clear();
	ContinuousNumber get_continuous_num;
	foreach([&ret, &get_continuous_num](int x){
		unsigned int continous_num = get_continuous_num(x);
		if (continous_num >= ret.size()) {
			ret.push_back(vector<int>());
		}
		ret[continous_num].push_back(x);
	});
	return ret;
}

void get_data_peaks(vector<unsigned int> &data,
		vector<unsigned int> &peaks, map<unsigned int, unsigned int> &peak_weight,
		unsigned int threshold, unsigned int kernel_size)
{
	int delta = kernel_size / 2;	
	vector<unsigned int> conv(data.size());
	fill(conv.begin(), conv.end(), 0);
	for (int i = 0; i < data.size(); i++) {
		int start = max(0, i - delta);
		int end = min((int)data.size() - 1, i + delta);
		for (int j = start; j <= end; j++) {
			conv[i] += data[j];
		}
	}
	auto foreach = [&conv, threshold](function<void(int)> cb){
		for (int i = 0; i < conv.size(); i++) {
			if (conv[i] > threshold) {
				cb((int)i);
			}
		}
	};
	auto ranges = ContinuousNumber::islands(foreach);
	peaks.resize(100);
	peaks.clear();
	for (int i = 0; i < ranges.size(); i++) {
		auto &range = ranges[i];
		unsigned max_index = range[0];	
		unsigned max_value = conv[range[0]];
		for (int j = 1; j < range.size(); j++) {
			if (max_value < conv[range[j]]) {
				max_index = range[j];
				max_value = conv[max_index];
			}
		}
		peaks.push_back(max_index);
		peak_weight[max_index] = max_value;
	}
}

void Nearest::init_data(vector<unsigned int> &points)
{
	vector<int> vec(points.size());
	vec.clear();
	for (auto itt = points.begin(); itt != points.end(); itt++) {
		vec.push_back((int)*itt);
	}
	init_data(vec);
}

void Nearest::init_data(vector<int> &points)
{
	vector<int> vec(points);
	sort(vec.begin(), vec.end());
	map<int, Integer<INT_MIN>> left;
	map<int, Integer<INT_MAX>> right;
	for (int i = 0; i + 1 < vec.size(); i++) {
		auto start = vec[i];
		auto end = vec[i + 1];
		auto middle = start / 2 + end / 2;
		left[end] = middle;
		right[start] = middle;
	}
	for (int i = 0; i < vec.size(); i++) {
		auto point = vec[i];
		assoc[tie(left[point], right[point])] = point;
	}
}

int Nearest::operator () (int search)
{
	return assoc[tie(search, search)];
}

int Nearest::diff(int search)
{
	int x = assoc[tie(search, search)];
	return x - search;
}

int Nearest::distance(int search)
{
	int d = diff(search);
	return d > 0 ? d : - d;
}

float get_line_confidence(
		std::vector<unsigned int> &from, Nearest &from_neighbor,
		std::vector<unsigned int> &to, Nearest &to_neighbor,
		std::map<point_t, float> &point_weight,	float s, int t)
{
	float confidence = 0.0;
	for (auto itt = from.begin(); itt != from.end(); itt++) {
		int y = (int)(s * (*itt) + t);
		int nearest_y = to_neighbor(y);
		int d = y - nearest_y;
		d = d > 0 ? d : -d;
		float w = point_weight[tie(*itt, nearest_y)];
		if (d < 4) {
			confidence += w * pow(0.8, d * d);
		}
	}

	for (auto itt = to.begin(); itt != to.end(); itt++) {
		int x = (int)((*itt - t) / s);
		int nearest_x = from_neighbor(x);
		int d = x - nearest_x;
		d = d > 0 ? d : -d;
		float w = point_weight[tie(nearest_x, *itt)];
		if (d < 4) {
			confidence += w * pow(0.8, d * d);
		}
	}
	auto point_count = from.size() + to.size();
	point_count = point_count ?: 1;
	return confidence;
//	return confidence / point_count;
}
