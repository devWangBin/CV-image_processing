#include <opencv2\opencv.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\features2d\features2d.hpp>
#include <opencv2\core\core.hpp>

using namespace std;
using namespace cv;

int main()
{
	Mat srcImg = imread("5555.jpg", 1);
	Mat imageGray;
	cvtColor(srcImg, imageGray, CV_RGB2GRAY);
	imageGray = ~imageGray;//对灰度图取反 

	GaussianBlur(imageGray, imageGray, Size(5, 5), 2); //滤波 
	threshold(imageGray, imageGray, 20, 200, CV_THRESH_BINARY); //阈值 
	imshow("threshold", imageGray);

	Mat distanceImg(imageGray.size(), CV_32FC1); //距离变换结果的Mat矩阵 
	distanceTransform(imageGray, distanceImg, CV_DIST_L2, 3); //距离变换 
	Mat dist;
	normalize(distanceImg, dist, 0, 1, NORM_MINMAX);
	imshow("distanceImg", dist);

	Mat distShowImg;
	distShowImg = Mat::zeros(imageGray.size(), CV_8UC1); //定义细化后的字符轮廓 
	float maxValue = 0;//距离变换矩阵中的最大值 
	for (int i = 0; i < distanceImg.rows; i++)
	{
		for (int j = 0; j < distanceImg.cols; j++)
		{
			if (distanceImg.at<float>(i, j) > maxValue)
			{
				maxValue = distanceImg.at<float>(i, j);//获取距离变换的极大值 
			}
		}
	}

	for (int i = 0; i < distanceImg.rows; i++)
	{
		for (int j = 0; j < distanceImg.cols; j++)
		{
			if (distanceImg.at<float>(i, j) > maxValue / 1.9)
			{
				distShowImg.at<unsigned char>(i, j) = 255;//符合距离大于最大值一定比例条件的点设为255 
			}
		}
	}
	//imshow("Source Image", srcImg);
	imshow("thinImg", distShowImg);

	Mat distShow;
	float maxValue2 = 0;
	distShow = Mat::zeros(imageGray.size(), CV_8UC1); //细化后的字符轮廓 
	Point Pt(0, 0);
	for (int i = 0; i < distanceImg.rows; i++)
	{
		for (int j = 0; j < distanceImg.cols; j++)
		{
			distShow.at<unsigned char>(i, j) = distanceImg.at<float>(i, j);
			if (distanceImg.at<float>(i, j) > maxValue2)
			{
				maxValue2 = distanceImg.at<float>(i, j);//获取距离变换的极大值 
				Pt = Point(j, i);//坐标 
			}
		}
	}
	circle(srcImg, Pt, maxValue2, Scalar(0, 0, 255), 3);
	circle(srcImg, Pt, 3, Scalar(0, 255, 0), 3);
	imshow("Source Image", srcImg);
	imshow("Thin Image", distShow);

	waitKey(0);
	return 0;
}
