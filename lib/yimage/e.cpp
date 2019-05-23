#include <boost/date_time.hpp>
#include "utils.hpp"

using namespace boost::property_tree;
using namespace boost::gregorian;
using namespace boost;
using namespace std;

int main (int argc, char ** argv)
{
	vector<int> v = {3, 4, 5};

	auto identity = fn_i(int);
	auto m = vector2map<int, int, int>(v, identity, identity);

	auto println = fn_println(int);
	auto get_first = fn_field(first, wt(pair<int, int>));

	auto print_first = fn_c(get_first, println);

//	for_each(m.begin(), m.end(), [println, get_first](auto item) {
//			println(get_first(item));
//	});

	for_each(m.begin(), m.end(), fn_c(
				fn_field(first, wt(pair<int, int>)),
				fn_println(int)
			));


//	type_of_member(first, wt(pair<int, int>)) good = 0;

//	cout << sizeof(member_type_of(pair<int, int>, first)) << endl;
//	cout << m.size() << endl;
//	string strJson = "{\"people\":[{\"firstName\":\"Brett\",\"lastName\":\"McLaughlin\",\"email\":\"aaaa\"},{\"firstName\":\"Brett\",\"lastName\":\"McLaughlin\",\"email\":\"aaaa\"}]}";
//	string stra,strc;
//	vector<string> vecStr;
//	ptree pt,p1,p2;
//	json_decode(strJson, pt);
//	p1 = pt.get_child("people");
//
//	for (ptree::iterator it = p1.begin(); it != p1.end(); ++it)
//	{
//			p2 = it->second; //first为空
//			stra = p2.get<string>("firstName");
//			vecStr.push_back(stra);   
//	}
//
//	{
//		stringstream outJson;
//		write_json<ptree>(outJson, pt, false);
//		string s;
//		outJson >> s;
//		cout << json_encode(pt) << endl;
//	}
//
//	for (vector<string>::iterator itt = vecStr.begin(); itt != vecStr.end(); itt ++)
//	{
//		cout << (*itt) << endl;
//	}
	return 0;
}

