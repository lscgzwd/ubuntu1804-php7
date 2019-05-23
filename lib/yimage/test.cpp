#include "yimage.hpp"
#include <iostream>
using namespace std;

void tYImageClassify(void)
{
	string str = "[\"./images/answer-card好.jpg\", \"./images/a.jpg\"]";
	string str0 = "[\"./images/answer-card好.jpg\", \"./images/a.jpg\", \"./images/raw-user-answer.jpg\", \"http://teacher.yijiafen.com/uploads/answer-cards/20160329/deskewed/7cd8378c-55d3-4294-83de-41cd6d579df4.jpg\", \"./images/answer-card0.jpg\"]";

	auto json = YImageClassify(str, str0); 
	cout << json << endl;
}


int main(int argc,char **argv)
{
	tImageVHLinesRotate();
//	tYImageClassify();
	return 0;
}
