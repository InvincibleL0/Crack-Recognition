#include "CrackProcess.h"

void crackImageProcess::addContrast(Mat& image)
{
	Mat lookUpTable(1, 256, CV_8U);
	double temp = pow(1.1, 5);
	uchar* p = lookUpTable.data;
	for (int i = 0; i < 256; ++i)
	{
		p[i] = saturate_cast<uchar>(i * temp);
	}
	LUT(image, lookUpTable, image);

}

void crackImageProcess::swapMat(Mat & src, Mat & dst)
{
	Mat tempImg = src;
	src = dst;
	dst = tempImg;
}

void crackImageProcess::findConnectedDomain(Mat & srcImg, vector<vector<Point>>& connectedDomains,
	int area, int WHRatio)
{
	Mat_<uchar> tempImg = (Mat_<uchar> &)srcImg;
	for (int i = 0; i < tempImg.rows; ++i)
	{
		uchar* row = tempImg.ptr(i);
		for (int j = 0; j < tempImg.cols; ++j) 
		{
			if (row[j] == 255) 
			{
				stack<Point> connectedPoints; //栈：先入后出
				vector<Point> domain;
				connectedPoints.push(Point(j, i));
				while (!connectedPoints.empty()) 
				{
					Point currentPoint = connectedPoints.top();
					domain.push_back(currentPoint);

					int colNum = currentPoint.x;
					int rowNum = currentPoint.y;

					tempImg.ptr(rowNum)[colNum] = 0;
					connectedPoints.pop();

					if (rowNum - 1 >= 0 && colNum - 1 >= 0 && tempImg.ptr(rowNum - 1)[colNum - 1] == 255) 
					{
						tempImg.ptr(rowNum - 1)[colNum - 1] = 0;
						connectedPoints.push(Point(colNum - 1, rowNum - 1));
					}
					if (rowNum - 1 >= 0 && tempImg.ptr(rowNum - 1)[colNum] == 255) {
						tempImg.ptr(rowNum - 1)[colNum] = 0;
						connectedPoints.push(Point(colNum, rowNum - 1));
					}
					if (rowNum - 1 >= 0 && colNum + 1 < tempImg.cols && tempImg.ptr(rowNum - 1)[colNum + 1] == 255) {
						tempImg.ptr(rowNum - 1)[colNum + 1] = 0;
						connectedPoints.push(Point(colNum + 1, rowNum - 1));
					}
					if (colNum - 1 >= 0 && tempImg.ptr(rowNum)[colNum - 1] == 255) {
						tempImg.ptr(rowNum)[colNum - 1] = 0;
						connectedPoints.push(Point(colNum - 1, rowNum));
					}
					if (colNum + 1 < tempImg.cols && tempImg.ptr(rowNum)[colNum + 1] == 255) {
						tempImg.ptr(rowNum)[colNum + 1] = 0;
						connectedPoints.push(Point(colNum + 1, rowNum));
					}
					if (rowNum + 1 < tempImg.rows && colNum - 1 > 0 && tempImg.ptr(rowNum + 1)[colNum - 1] == 255) {
						tempImg.ptr(rowNum + 1)[colNum - 1] = 0;
						connectedPoints.push(Point(colNum - 1, rowNum + 1));
					}
					if (rowNum + 1 < tempImg.rows && tempImg.ptr(rowNum + 1)[colNum] == 255) {
						tempImg.ptr(rowNum + 1)[colNum] = 0;
						connectedPoints.push(Point(colNum, rowNum + 1));
					}
					if (rowNum + 1 < tempImg.rows && colNum + 1 < tempImg.cols && tempImg.ptr(rowNum + 1)[colNum + 1] == 255) {
						tempImg.ptr(rowNum + 1)[colNum + 1] = 0;
						connectedPoints.push(Point(colNum + 1, rowNum + 1));
					}
				}



				if (domain.size() > area) {
					RotatedRect rect = minAreaRect(domain);
					float width = rect.size.width;
					float height = rect.size.height;
					if (width < height) {
						float temp = width;
						width = height;
						height = temp;
					}
					if (width > height * WHRatio && width >100) {
						for (auto cit = domain.begin(); cit != domain.end(); ++cit) {
							tempImg.ptr(cit->y)[cit->x] = 250;
						}
						connectedDomains.push_back(domain);
					}
				}
			}
		}
	}

	binaryzation(srcImg);

}
void crackImageProcess::binaryzation(Mat & srcImg)
{
	Mat lookUpTable(1, 256, CV_8U, Scalar(255));
	lookUpTable.data[0] = 0;
	LUT(srcImg, lookUpTable, srcImg);

	return;
}

void crackImageProcess::thinImage(Mat & srcImg)
{
	vector<Point> deleteList;
	int neighbourhood[9];
	int nl = srcImg.rows;
	int nc = srcImg.cols;
	bool inOddIterations = true;
	while (true)
	{
		for (int j = 1; j < (nl - 1); j++) {
			uchar* data_last = srcImg.ptr<uchar>(j - 1);
			uchar* data = srcImg.ptr<uchar>(j);
			uchar* data_next = srcImg.ptr<uchar>(j + 1);
			for (int i = 1; i < (nc - 1); i++) {
				if (data[i] == 255) {
					int whitePointCount = 0;
					neighbourhood[0] = 1;
					if (data_last[i] == 255) neighbourhood[1] = 1;
					else  neighbourhood[1] = 0;
					if (data_last[i + 1] == 255) neighbourhood[2] = 1;
					else  neighbourhood[2] = 0;
					if (data[i + 1] == 255) neighbourhood[3] = 1;
					else  neighbourhood[3] = 0;
					if (data_next[i + 1] == 255) neighbourhood[4] = 1;
					else  neighbourhood[4] = 0;
					if (data_next[i] == 255) neighbourhood[5] = 1;
					else  neighbourhood[5] = 0;
					if (data_next[i - 1] == 255) neighbourhood[6] = 1;
					else  neighbourhood[6] = 0;
					if (data[i - 1] == 255) neighbourhood[7] = 1;
					else  neighbourhood[7] = 0;
					if (data_last[i - 1] == 255) neighbourhood[8] = 1;
					else  neighbourhood[8] = 0;
					for (int k = 1; k < 9; k++) {
						whitePointCount = whitePointCount + neighbourhood[k];
					}
					if ((whitePointCount >= 2) && (whitePointCount <= 6)) {
						int ap = 0;
						if ((neighbourhood[1] == 0) && (neighbourhood[2] == 1)) ap++;
						if ((neighbourhood[2] == 0) && (neighbourhood[3] == 1)) ap++;
						if ((neighbourhood[3] == 0) && (neighbourhood[4] == 1)) ap++;
						if ((neighbourhood[4] == 0) && (neighbourhood[5] == 1)) ap++;
						if ((neighbourhood[5] == 0) && (neighbourhood[6] == 1)) ap++;
						if ((neighbourhood[6] == 0) && (neighbourhood[7] == 1)) ap++;
						if ((neighbourhood[7] == 0) && (neighbourhood[8] == 1)) ap++;
						if ((neighbourhood[8] == 0) && (neighbourhood[1] == 1)) ap++;
						if (ap == 1) {
							if (inOddIterations && (neighbourhood[3] * neighbourhood[5] * neighbourhood[7] == 0)
								&& (neighbourhood[1] * neighbourhood[3] * neighbourhood[5] == 0)) {
								deleteList.push_back(Point(i, j));
							}
							else if (!inOddIterations && (neighbourhood[1] * neighbourhood[5] * neighbourhood[7] == 0)
								&& (neighbourhood[1] * neighbourhood[3] * neighbourhood[7] == 0)) {
								deleteList.push_back(Point(i, j));
							}
						}
					}
				}
			}
		}
		if (deleteList.size() == 0)
			break;
		for (size_t i = 0; i < deleteList.size(); i++) {
			Point tem;
			tem = deleteList[i];
			uchar* data = srcImg.ptr<uchar>(tem.y);
			data[tem.x] = 0;
		}
		deleteList.clear();

		inOddIterations = !inOddIterations;
	}


}

void crackImageProcess::getWhitePoints(Mat & srcImg, vector<Point>& domain)
{
	domain.clear();
	Mat_<uchar> tempImg = (Mat_<uchar> &)srcImg;
	for (int i = 0; i < tempImg.rows; i++) {
		uchar * row = tempImg.ptr<uchar>(i);
		for (int j = 0; j < tempImg.cols; ++j) {
			if (row[j] != 0)
				domain.push_back(Point(j, i));
		}
	}
}

cv::Point crackImageProcess::calInfoPosition(int imgRows, int imgCols, int padding, const std::vector<cv::Point>& domain)
{
	long xSum = 0;
	long ySum = 0;
	for (auto it = domain.cbegin(); it != domain.cend(); ++it) {
		xSum += it->x;
		ySum += it->y;
	}
	int x = 0;
	int y = 0;
	x = (int)(xSum / domain.size());
	y = (int)(ySum / domain.size());
	if (x < padding)
		x = padding;
	if (x > imgCols - padding)
		x = imgCols - padding;
	if (y < padding)
		y = padding;
	if (y > imgRows - padding)
		y = imgRows - padding;

	return cv::Point(x, y);
}

