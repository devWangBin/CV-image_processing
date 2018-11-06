#include<cv.h>
#include<highgui.h>
#include<iostream>
#include<io.h>
#include<opencv2\opencv.hpp>   

#include"ransac_circle2d.h"

using namespace std;
using namespace cv;

bool testCircle2dMulti(){

	cv::Mat src = cv::imread("circlesTest.bmp", CV_LOAD_IMAGE_GRAYSCALE);

	if (src.empty())
		return false;

	Mat inImg, showMat;
	cvtColor(src, showMat, CV_GRAY2BGR);
	threshold(src, inImg, 10, 255, CV_THRESH_BINARY_INV);
	
	std::vector<sac::Point2D> pCloud2D;

	for (int i = 0; i < inImg.rows; i++)
	{
		uchar* p = inImg.ptr<uchar>(i);
		for (int j = 0; j < inImg.cols; j++)
		{
			if (p[j] != 0)
				pCloud2D.push_back(sac::Point2D(j, i));
		}
	}

	double specRadius = 100;

	sac::ransacModelCircle2D circle2D;
	std::vector<int> inliers;
	sac::ModelCoefficient parameter;
	circle2D.setDistanceThreshold(5);
	circle2D.setMaxIterations(2500);
	//circle2D.setSpecificRadius(specRadius, 0.2);

	while (pCloud2D.size() > 500)
	{
		circle2D.setInputCloud(pCloud2D);
		circle2D.computeModel();
		circle2D.getInliers(inliers);
		circle2D.getModelCoefficients(parameter);

		if (inliers.size() < specRadius * 2 * CV_PI)
			break;

		Point cp((int)parameter.modelParam[0], (int)parameter.modelParam[1]);
		int radius = (int)parameter.modelParam[2];
		circle(showMat, cp, radius, Scalar(0, 255, 0), 2, 8);

		/*imshow("circles", showMat);
		waitKey(1000);*/

		cout << "Parameter of 2D line: < " << parameter.modelParam[0] << ", " <<
			parameter.modelParam[1] << " >---" << parameter.modelParam[2] << endl;

		circle2D.removeInliders(pCloud2D, inliers);
	}
	imshow("circles", showMat);
	waitKey(0);

	cout <<"finished!!!"<< endl;

	return true;
}


//边缘检测
int main()
{
	//Mat img = imread("circlesTest.bmp");
	Mat img = imread("9999.jpg");
	imshow("原始图", img);
	Mat DstPic, edge, grayImage;
	//创建与src同类型和同大小的矩阵
	DstPic.create(img.size(), img.type());
	DstPic = Scalar::all(0);
	//将原始图转化为灰度图
	cvtColor(img, grayImage, COLOR_BGR2GRAY);
	//先使用3*3内核来降噪
	blur(grayImage, edge, Size(5, 5));
	//运行canny算子
	Canny(edge, edge, 70, 210, 3);

	Mat smallEdge;
	int width = static_cast<float>(edge.cols*0.5);
	//定义想要扩大或者缩小后的宽度，src.cols为原图像的宽度，乘以80%则得到想要的大小，并强制转换成float型
	int height = static_cast<float>(edge.rows*0.5);
	//定义想要扩大或者缩小后的高度，src.cols为原图像的高度，乘以80%则得到想要的大小，并强制转换成float型

	resize(edge,smallEdge, cv::Size(width, height));
	
	imshow("smallEdge", smallEdge);
	imshow("边缘提取效果", edge);
	waitKey(0);
	//使用Canny算子输出的边缘图g_cannyDetectedEdges作为掩码，来将原图g_srcImage拷到目标图g_dstImage中  
	img.copyTo(DstPic,edge);
	//显示效果图   
	imshow("【效果图】Canny边缘检测2", DstPic);
	waitKey(0);
	//显示Random sample consensus算法的圆检测结果
	//25秒
	testCircle2dMulti();
	
}