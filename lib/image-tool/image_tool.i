%module image_tool
%include <std_string.i>

%{

std::string classify_by_line(std::string templateStr, std::string str);
std::string extract_black_rectangle(std::string img_base64, int threshold);

%}
extern std::string classify_by_line(std::string templateStr, std::string str);
extern std::string extract_black_rectangle(std::string img_base64, int threshold);
