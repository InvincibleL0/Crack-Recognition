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
处理流程如下：

图像灰度化
增加对比度
Canny边缘检测
用形态学连接临近裂缝
找出所有连通域，删除非裂缝噪点区域
对每个连通域提取骨架，测量长度
*/
//可以观察每一步的处理结果   VS2015立面安装ImageWath插件，显示每一步的处理结果


//相机信息
struct CammerInfo
{
	double  pixelsize;        //像元大小
	double  focalLength;    //相机焦距
	double   workLength;    //工作距离
	double  resolution;       //分辨率
};


void  main()
{
	//1. 读取图片
	Mat   img = imread("./DJI_0039.JPG");   //每次只需要修改图片路径即可
	//转成灰度图像
	cvtColor(img, img, CV_BGR2GRAY);
	imwrite("01_灰度化.jpg", img);


	Mat  dst = img.clone();
	cout << "灰度化完成" << endl;
	//利用LUT函数完成图像对比度增加

	craProcess.addContrast(img);
	cout << "增加对比度完成" << endl;
	imwrite("02_增加对比度.jpg", img);


	//canny算法进行边缘检测
	Canny(img, img, 40, 150);
	imwrite("03_边缘检测.jpg", img);

	cout << "边缘检测完成" << endl;

	//2.  形态学操作：对裂缝区域进行筛选
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	dilate(img, img, kernel);
	morphologyEx(img, img, CV_MOP_CLOSE, kernel, Point(-1, -1), 2);
	
	imwrite("04_形态学操作.jpg", img);
	//找出所有连通域，删除非裂缝噪点区域
	vector<vector<Point>> connectedDomains;
	/*
	关键步骤：findConnectedDomain函数的作用：
	找出符合条件的联通区域(即裂缝)，利用裂缝外接矩形的长度和宽度以及裂缝的面积大小加以限制

	*/



	craProcess.findConnectedDomain(img, connectedDomains, 50, 1.5);
	//形态学运算：连接断掉的裂缝
	kernel = getStructuringElement(MORPH_ELLIPSE, Size(7, 7));
	morphologyEx(img, img, CV_MOP_CLOSE, kernel, Point(-1, -1), 5);


	connectedDomains.clear();
	//再一次对裂缝进行筛选
	craProcess.findConnectedDomain(img, connectedDomains, 50, 1.5);
	//然后形态学连接断掉的裂缝
	kernel = getStructuringElement(MORPH_CROSS, Size(3,3));
	morphologyEx(img, img, CV_MOP_OPEN, kernel);

	kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	erode(img, img, kernel);
	imwrite("05_腐蚀.jpg", img);
	connectedDomains.clear();
	//最后筛选出符合条件的裂缝
	craProcess.findConnectedDomain(img, connectedDomains, 50, 1.5);

	cout << "开始测量" << endl;
	cout << "连通域数量：" << connectedDomains.size() << endl;
	Mat lookUpTable(1, 256, CV_8U, Scalar(0));
	

	//crackInfos是保存裂缝信息的对象 ：裂缝的长度和平均宽度 
	vector<CrackInfo> crackInfos;
	for (auto domain_it = connectedDomains.begin(); domain_it != connectedDomains.end(); ++domain_it) {
		LUT(img, lookUpTable, img);
		for (auto point_it = domain_it->cbegin(); point_it != domain_it->cend(); ++point_it) {
			img.ptr<uchar>(point_it->y)[point_it->x] = 255;
		}
		double area = (double)domain_it->size();
		//对每个连通域提取骨架，测量长度和宽度
		craProcess.thinImage(img);  //对裂缝区域进行骨架化
		craProcess.getWhitePoints(img, *domain_it);
		long length = (long)domain_it->size();
		//计算裂缝的中心位置
		Point position = craProcess.calInfoPosition(img.rows, img.cols, 50, *domain_it);
		CrackInfo  crainfo;
		//裂缝中心的x坐标
		crainfo.X = position.x;
		//裂缝中心的y坐标
		crainfo.Y = position.y;
		//裂缝的像素长度
		crainfo.Length = length;
		//裂缝的像素宽度=裂缝的像素面积/长度
		crainfo.Width = (float)(area / length);
		//裂缝的面积
		crainfo.Area = area;

		crackInfos.push_back(crainfo);
	}

	cout << "一共检测出来的裂缝的数量：" << crackInfos.size() << endl;
	LUT(img, lookUpTable, img);
	for (auto domain_it = connectedDomains.cbegin(); domain_it != connectedDomains.cend(); ++domain_it) {
		for (auto point_it = domain_it->cbegin(); point_it != domain_it->cend(); ++point_it) {
			img.ptr<uchar>(point_it->y)[point_it->x] = 255;
		}
	}

	//在输入图像检测出的裂缝用红色标注
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
	//保存检测结果图像
	imwrite("06_result.jpg", draw_img);


	ofstream   fout("result.txt");
	if (!fout.is_open())
	{
		cout << "打开文件失败！" << endl;
		return;
	}

	//输出检测结果: 单位均为像素尺度（pixel）
	fout << "保存像素单位的裂缝信息：" << endl;
	char buf[1024] = {'0'};
	for (size_t i = 0; i < crackInfos.size(); i++)
	{
		CrackInfo   info = crackInfos[i];
		
		sprintf(buf, "第%d个裂缝的信息：位置(%d,%d)  面积：%.02f,长度: %.02f  宽度: %.02f", i+1 ,
			info.X, info.Y, info.Area, info.Length, info.Width);
		cout << buf << endl;
		fout << buf << endl;

	}

	//计算相机的分辨率
	CammerInfo   camInfo;
	camInfo.focalLength = 0.88; //cm
	camInfo.pixelsize = 0.000214;  //cm
	camInfo.workLength = 400;  //cm
	camInfo.resolution = (camInfo.pixelsize / camInfo.focalLength)*camInfo.workLength;


	double  r = camInfo.resolution;
	fout<< endl << endl << endl;
	fout << "保存实际单位的裂缝信息：" << endl;



	for (size_t i = 0; i < crackInfos.size(); i++)
	{
		CrackInfo   info = crackInfos[i];
		sprintf(buf, "第%d个裂缝的信息：位置(%d,%d)  面积：%.02f,长度: %.02f  宽度: %.02f", i + 1,
			info.X*r, info.Y*r, info.Area*r*r, info.Length*r, info.Width*r);
		fout << buf << endl;
		cout << buf << endl;

	}

	



	fout.close();



	cout << "prcocess  sucess!" << endl;
	waitKey(0);
}
