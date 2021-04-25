#include<iostream>
#include<stack>
#include<vector>
#include<opencv2/opencv.hpp>
using namespace cv;
using namespace std;


class  crackImageProcess
{

public:
	//增加图像对比度
	void addContrast(Mat & images);
	//交换两个图像
	void swapMat(Mat& src, Mat&  dst);
	//筛选符合条件的裂缝
	void  findConnectedDomain(Mat & srcImg, vector<vector<Point>>& connectedDomains, int area, int WHRatio);
	//二值化
	void binaryzation(Mat& srcImg);
	//对联通区域骨架化
	void thinImage(Mat & srcImg);
	//计算裂缝的长度即像素的点数
	void getWhitePoints(Mat &srcImg, vector<Point>& domain);
	//计算裂缝的中心
	cv::Point calInfoPosition(int imgRows, int imgCols, int padding, const std::vector<cv::Point>& domain);



};

