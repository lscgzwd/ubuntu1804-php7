#include <iostream>
#include <functional>
#include <algorithm>
#include <boost/foreach.hpp>
#include <map>
#include <vector>
#include "utils.hpp"
#include "fn.hpp"
using namespace std;

int main(int argc, char ** argv)
{
	string str = "[\"1\", \"2\", \"3\", 4]";
	auto vec = json2vector(str);
	for_each(vec.begin(), vec.end(), fn_println<string>());
	return 0;
}
