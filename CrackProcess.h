#include<iostream>
#include<stack>
#include<vector>
#include<opencv2/opencv.hpp>
using namespace cv;
using namespace std;


class  crackImageProcess
{

public:
	//����ͼ��Աȶ�
	void addContrast(Mat & images);
	//��������ͼ��
	void swapMat(Mat& src, Mat&  dst);
	//ɸѡ�����������ѷ�
	void  findConnectedDomain(Mat & srcImg, vector<vector<Point>>& connectedDomains, int area, int WHRatio);
	//��ֵ��
	void binaryzation(Mat& srcImg);
	//����ͨ����Ǽܻ�
	void thinImage(Mat & srcImg);
	//�����ѷ�ĳ��ȼ����صĵ���
	void getWhitePoints(Mat &srcImg, vector<Point>& domain);
	//�����ѷ������
	cv::Point calInfoPosition(int imgRows, int imgCols, int padding, const std::vector<cv::Point>& domain);



};

