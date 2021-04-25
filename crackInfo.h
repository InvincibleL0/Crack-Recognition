#pragma once
#include<iostream>
#include<stack>
#include<vector>
#include<opencv2/opencv.hpp>
using namespace cv;
using namespace std;

//描述像素信息的类
namespace Custom {
	class CrackInfo
	{	
	public:
		CrackInfo();
		~CrackInfo();
		cv::Point Position;
		double  Area;//面积
		double Length;//长度
		double  Width; //宽度
		int X; //中心的x坐标
		int Y; //中心的y坐标
	};
}
