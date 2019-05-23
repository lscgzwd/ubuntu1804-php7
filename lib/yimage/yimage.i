%module yimage
%include <std_string.i>

%{

#include "yimage.hpp"
std::string YImageClassify(std::string templateStr, std::string str);
std::string YImageEntropy(std::string str);

%}
extern std::string YImageClassify(std::string templateStr, std::string str);
extern std::string YImageEntropy(std::string str);
