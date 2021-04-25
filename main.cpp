#include<iostream>
#include<opencv2/opencv.hpp>
#include"CrackProcess.h"
#include"crackInfo.h"
using namespace cv;
using namespace std;
using namespace Custom;
crackImageProcess    craProcess;
RNG  rng(12345);

/*
�����������£�

ͼ��ҶȻ�
���ӶԱȶ�
Canny��Ե���
����̬ѧ�����ٽ��ѷ�
�ҳ�������ͨ��ɾ�����ѷ��������
��ÿ����ͨ����ȡ�Ǽܣ���������
*/
//���Թ۲�ÿһ���Ĵ�����   VS2015���氲װImageWath�������ʾÿһ���Ĵ�����


//�����Ϣ
struct CammerInfo
{
	double  pixelsize;        //��Ԫ��С
	double  focalLength;    //�������
	double   workLength;    //��������
	double  resolution;       //�ֱ���
};


void  main()
{
	//1. ��ȡͼƬ
	Mat   img = imread("./DJI_0039.JPG");   //ÿ��ֻ��Ҫ�޸�ͼƬ·������
	//ת�ɻҶ�ͼ��
	cvtColor(img, img, CV_BGR2GRAY);
	imwrite("01_�ҶȻ�.jpg", img);


	Mat  dst = img.clone();
	cout << "�ҶȻ����" << endl;
	//����LUT�������ͼ��Աȶ�����

	craProcess.addContrast(img);
	cout << "���ӶԱȶ����" << endl;
	imwrite("02_���ӶԱȶ�.jpg", img);


	//canny�㷨���б�Ե���
	Canny(img, img, 40, 150);
	imwrite("03_��Ե���.jpg", img);

	cout << "��Ե������" << endl;

	//2.  ��̬ѧ���������ѷ��������ɸѡ
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	dilate(img, img, kernel);
	morphologyEx(img, img, CV_MOP_CLOSE, kernel, Point(-1, -1), 2);
	
	imwrite("04_��̬ѧ����.jpg", img);
	//�ҳ�������ͨ��ɾ�����ѷ��������
	vector<vector<Point>> connectedDomains;
	/*
	�ؼ����裺findConnectedDomain���������ã�
	�ҳ�������������ͨ����(���ѷ�)�������ѷ���Ӿ��εĳ��ȺͿ���Լ��ѷ�������С��������

	*/



	craProcess.findConnectedDomain(img, connectedDomains, 50, 1.5);
	//��̬ѧ���㣺���Ӷϵ����ѷ�
	kernel = getStructuringElement(MORPH_ELLIPSE, Size(7, 7));
	morphologyEx(img, img, CV_MOP_CLOSE, kernel, Point(-1, -1), 5);


	connectedDomains.clear();
	//��һ�ζ��ѷ����ɸѡ
	craProcess.findConnectedDomain(img, connectedDomains, 50, 1.5);
	//Ȼ����̬ѧ���Ӷϵ����ѷ�
	kernel = getStructuringElement(MORPH_CROSS, Size(3,3));
	morphologyEx(img, img, CV_MOP_OPEN, kernel);

	kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	erode(img, img, kernel);
	imwrite("05_��ʴ.jpg", img);
	connectedDomains.clear();
	//���ɸѡ�������������ѷ�
	craProcess.findConnectedDomain(img, connectedDomains, 50, 1.5);

	cout << "��ʼ����" << endl;
	cout << "��ͨ��������" << connectedDomains.size() << endl;
	Mat lookUpTable(1, 256, CV_8U, Scalar(0));
	

	//crackInfos�Ǳ����ѷ���Ϣ�Ķ��� ���ѷ�ĳ��Ⱥ�ƽ����� 
	vector<CrackInfo> crackInfos;
	for (auto domain_it = connectedDomains.begin(); domain_it != connectedDomains.end(); ++domain_it) {
		LUT(img, lookUpTable, img);
		for (auto point_it = domain_it->cbegin(); point_it != domain_it->cend(); ++point_it) {
			img.ptr<uchar>(point_it->y)[point_it->x] = 255;
		}
		double area = (double)domain_it->size();
		//��ÿ����ͨ����ȡ�Ǽܣ��������ȺͿ��
		craProcess.thinImage(img);  //���ѷ�������йǼܻ�
		craProcess.getWhitePoints(img, *domain_it);
		long length = (long)domain_it->size();
		//�����ѷ������λ��
		Point position = craProcess.calInfoPosition(img.rows, img.cols, 50, *domain_it);
		CrackInfo  crainfo;
		//�ѷ����ĵ�x����
		crainfo.X = position.x;
		//�ѷ����ĵ�y����
		crainfo.Y = position.y;
		//�ѷ�����س���
		crainfo.Length = length;
		//�ѷ�����ؿ��=�ѷ���������/����
		crainfo.Width = (float)(area / length);
		//�ѷ�����
		crainfo.Area = area;

		crackInfos.push_back(crainfo);
	}

	cout << "һ�����������ѷ��������" << crackInfos.size() << endl;
	LUT(img, lookUpTable, img);
	for (auto domain_it = connectedDomains.cbegin(); domain_it != connectedDomains.cend(); ++domain_it) {
		for (auto point_it = domain_it->cbegin(); point_it != domain_it->cend(); ++point_it) {
			img.ptr<uchar>(point_it->y)[point_it->x] = 255;
		}
	}

	//������ͼ��������ѷ��ú�ɫ��ע
	vector<vector<Point>>	contours;
	vector<Vec4i>	hieracy;
	Mat draw_img = dst.clone();
	cvtColor(draw_img, draw_img, CV_GRAY2BGR);
	findContours(img, contours, hieracy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	Scalar  color;
	for (size_t i = 0; i < contours.size(); i++)
	{

		color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(draw_img, contours, i, color, 2, 8, hieracy);
		
	}
	color = Scalar(0, 0, 255);
	for (size_t i = 0; i < crackInfos.size(); i++)
	{
		CrackInfo   info = crackInfos[i];
		Point2f   pt = Point2f(info.X, info.Y);
		
		putText(draw_img, format("%d", i + 1), pt, FONT_HERSHEY_PLAIN, 5, color, 2);
	}
	//��������ͼ��
	imwrite("06_result.jpg", draw_img);


	ofstream   fout("result.txt");
	if (!fout.is_open())
	{
		cout << "���ļ�ʧ�ܣ�" << endl;
		return;
	}

	//��������: ��λ��Ϊ���س߶ȣ�pixel��
	fout << "�������ص�λ���ѷ���Ϣ��" << endl;
	char buf[1024] = {'0'};
	for (size_t i = 0; i < crackInfos.size(); i++)
	{
		CrackInfo   info = crackInfos[i];
		
		sprintf(buf, "��%d���ѷ����Ϣ��λ��(%d,%d)  �����%.02f,����: %.02f  ���: %.02f", i+1 ,
			info.X, info.Y, info.Area, info.Length, info.Width);
		cout << buf << endl;
		fout << buf << endl;

	}

	//��������ķֱ���
	CammerInfo   camInfo;
	camInfo.focalLength = 0.88; //cm
	camInfo.pixelsize = 0.000214;  //cm
	camInfo.workLength = 400;  //cm
	camInfo.resolution = (camInfo.pixelsize / camInfo.focalLength)*camInfo.workLength;


	double  r = camInfo.resolution;
	fout<< endl << endl << endl;
	fout << "����ʵ�ʵ�λ���ѷ���Ϣ��" << endl;



	for (size_t i = 0; i < crackInfos.size(); i++)
	{
		CrackInfo   info = crackInfos[i];
		sprintf(buf, "��%d���ѷ����Ϣ��λ��(%d,%d)  �����%.02f,����: %.02f  ���: %.02f", i + 1,
			info.X*r, info.Y*r, info.Area*r*r, info.Length*r, info.Width*r);
		fout << buf << endl;
		cout << buf << endl;

	}

	



	fout.close();



	cout << "prcocess  sucess!" << endl;
	waitKey(0);
}
