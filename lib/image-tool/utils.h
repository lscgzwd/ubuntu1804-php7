#ifndef _UTILS_H_
#define _UTILS_H_
#include <climits>
#include <vector>
#include <map>
#include <tuple>
#include <functional>
struct ContinuousNumber
{
	unsigned int continuous_number = 0;
	int last = INT_MIN;

	ContinuousNumber() {}
	unsigned int operator () (int x);

	static std::vector<std::vector<int>>
		islands(std::function<void(std::function<void(int)>)>);
};

void get_data_peaks(std::vector<unsigned int> &data,
		std::vector<unsigned int> &peaks, std::map<unsigned int, unsigned int> &peak_weight,
		unsigned int threshold = 200, unsigned int kernel_size = 3);

/*
 * 带默认值的整型
 */
template <int N = 0, typename T = int>
struct Integer
{
        T v = N;
        Integer() {}
        Integer(T v) : v(v) {}
        Integer<N, T> operator = (T x)
        {
                v = x;
                return *this;
        }
        Integer<N, T> operator = (Integer<N, T> x)
        {
                return (*this = x.v);
        }
        operator T ()
        {
                return v;
        }
};

struct cmp_tuple_range
{
    bool operator () (const std::tuple<int, int> &a, const std::tuple<int, int> &b) 
    {   
        return std::get<0>(a) < std::get<0>(b) && std::get<1>(a) < std::get<1>(b);
    }   
};

struct Nearest
{
	std::map<std::tuple<int, int>, Integer<INT_MAX>, cmp_tuple_range> assoc;

	Nearest() {}
	Nearest(std::vector<int> &points) {
		init_data(points);
	}
	int operator () (int search);
	void init_data(std::vector<int> &points);
	void init_data(std::vector<unsigned int> &points);
	int diff(int search);
	int distance(int search);
};
typedef std::tuple<unsigned int, unsigned int> point_t;

float get_line_confidence(
		std::vector<unsigned int> &from, Nearest &from_neighbor,
		std::vector<unsigned int> &to, Nearest &to_neighbor,
		std::map<point_t, float> &point_weight, float s, int t);
#endif/*_UTILS_H_*/
