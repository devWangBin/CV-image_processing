#include<iostream>  
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;


int main()
{
	Mat img = imread("a.png", -1);
	Mat back_img = imread("back_a.png", -1);
	Mat mat(img.rows, img.cols, CV_8UC4);//#define CV_8UC4 CV_MAKETYPE(CV_8U,4)可以创建-----8位无符号的四通道---带透明色的RGB图像 
	for (int i = 0; i < img.rows; ++i)
	{
		for (int j = 0; j < img.cols; ++j)
		{
			double temp = img.at<Vec4b>(i, j)[3] / 255.00;
			mat.at<Vec4b>(i, j)[0] = (1 - temp)*back_img.at<Vec4b>(i, j)[0] + temp * img.at<Vec4b>(i, j)[0];
			mat.at<Vec4b>(i, j)[1] = (1 - temp)*back_img.at<Vec4b>(i, j)[1] + temp * img.at<Vec4b>(i, j)[1];
			mat.at<Vec4b>(i, j)[2] = (1 - temp)*back_img.at<Vec4b>(i, j)[2] + temp * img.at<Vec4b>(i, j)[2];
			mat.at<Vec4b>(i, j)[3] = (1 - temp)*back_img.at<Vec4b>(i, j)[3] + temp * img.at<Vec4b>(i, j)[3];
		}	
	}
	namedWindow("alpha混合图像");
	imshow("alpha混合图像", mat);
	waitKey();
	return 0;
}