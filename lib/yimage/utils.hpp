#ifndef UTILS_HPP
#define UTILS_HPP
#include <json/json.h>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include "fn.hpp"

std::string json_encode(const Json::Value &jv)
{
	Json::FastWriter fastWriter;
	std::stringstream outJson;
	outJson << fastWriter.write(jv);
	std::string s;
	s = outJson.str();
	return s;
}

bool json_decode(const std::string & json, Json::Value &root)
{
	std::stringstream ss;
	ss << json;
	Json::Reader reader;
	return reader.parse(ss, root, false);
}

std::vector<std::string>
json2vector(std::string str)
{
	std::vector<std::string> ret;
	Json::Value root;
	json_decode(str, root);
	if (root.type() == Json::ValueType::arrayValue) {
		for (int i = 0; i < root.size(); i++) {
			auto item = root[i];
			if (item.type() == Json::ValueType::stringValue) {
				ret.push_back(item.asString());
			}
		}
	}
	return ret;
}

template <typename T, typename K, typename V>
std::map<K, V> vector2map(
		std::vector<T> &v,
		std::function<K(T)> &getKey,
		std::function<V(T)> &getValue)
{
	std::map<K, V> ret;
	for_each(v.begin(), v.end(), [&ret, &getKey, &getValue](T item){
			ret[getKey(item)] = getValue(item);
	});
	return ret;
}

//wrap type
#define wt __typeof__

#define FN_identity(name, type)																		\
	std::function<type(type) > name = [](type x){ return x; }

#define fn_i(type)																													\
({                                                                          \
	std::function<type(type) > ___identity = [](type x){ return x; };         \
	___identity;                                                              \
})

#define type_of_member(member, struct_type)				\
	__typeof__(((struct_type *) 0)->member)

#define fn_field(member, struct_type)																						\
({                                                                                  \
	typedef type_of_member(member, struct_type)	member_type;													\
	std::function<member_type(struct_type)> __get_field = 														\
		[](struct_type s){ return s.member; };                                          \
	__get_field;                                                                      \
})

#define fn_println(type)																														\
({                                                                                  \
 std::function<void(type)> ___println =                                             \
 	[](type x) { std::cout << x << endl; };                                           \
	___println;                                                                       \
})

#endif/*UTILS_HPP*/
