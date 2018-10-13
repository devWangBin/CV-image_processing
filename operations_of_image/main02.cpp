#include<iostream>  
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;


int mainff()
{
	Mat img = imread("a.png", -1);//-1使能读取第4个RGBA中的A通道，即为alpha通道
	Mat mat(img.rows, img.cols, CV_8UC1);//可以创建----8位无符号的单通道---灰度图片------grayImg
	for (int i = 0; i < img.rows; ++i)
	{
		for (int j = 0; j < img.cols; ++j)
		{
			mat.at<uchar>(i, j) = img.at<Vec4b>(i, j)[3];
		}
	}
	namedWindow("alpha通道图像");
	imshow("alpha通道图像", mat);
	waitKey();
	return 0;
}