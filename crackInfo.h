#pragma once
#include<iostream>
#include<stack>
#include<vector>
#include<opencv2/opencv.hpp>
using namespace cv;
using namespace std;

//����������Ϣ����
namespace Custom {
	class CrackInfo
	{	
	public:
		CrackInfo();
		~CrackInfo();
		cv::Point Position;
		double  Area;//���
		double Length;//����
		double  Width; //���
		int X; //���ĵ�x����
		int Y; //���ĵ�y����
	};
}
