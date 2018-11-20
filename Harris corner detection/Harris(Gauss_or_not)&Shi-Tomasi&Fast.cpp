#include<opencv2/opencv.hpp>
#include<vector>
#include <iostream>

using namespace cv;
using namespace std;

//全局变量
Mat src, src_gray;
int thresh = 200;
double myHarrisThrehold_ave = 10000000000;
double myHarrisThrehold_Gauss = 1000000000000;

void Shi_Tomasi_demo(Mat& srcImage, int max_cornerNum);
void Fast_demo(Mat& srcImage, int threshold);
Mat computeImage(Mat& ixx, Mat& iyy, Mat& ixy, int wsize);
void mixP(Mat& point, Mat& img, int psize);
Mat filterR(Mat& img, int wsize);
Mat computeImage(Mat& ix, Mat& iy, int wsize, int para);
void sobelGradient(Mat& img, Mat& dst, int para);
void cornerHarris_demo();
void myHarrisCorner_Gauss(Mat &srcImg);
void myHarrisCorner_ave(Mat &srcImg);

int main()
{	
	src = imread("2222.jpg",1);
	if (src.empty())
	{
		cout << "Could not open or find the image!\n" << endl;
		cout << "Usage:  <Input image>" << endl;
		return -1;
	}
	imshow("src_image", src);
	cvtColor(src, src_gray, COLOR_BGR2GRAY);

	//fast检测示例
	Fast_demo(src, 60);

	//ST角点检测示例
	Shi_Tomasi_demo(src, 200);
	//imshow("graYYYY", src_gray);
	cornerHarris_demo();
	myHarrisCorner_ave(src);
	myHarrisCorner_Gauss(src);
	waitKey(0);
	return 0;
}

void Shi_Tomasi_demo(Mat& srcImage,int max_cornerNum){

	Mat copyImage;
	copyImage = srcImage.clone();

	Mat imageGray;
	cvtColor(copyImage, imageGray, CV_BGR2GRAY);

	//goodFeaturesToTrack函数参数设置
	vector<Point2f> corner;
	double qualityLevel = 0.01;   
	double minDistance = 10;	   
	int blockSize = 3;	   
	bool useHarrisDetector = false;	   
	double k = 0.04;	   
	//调用函数goodFeaturesToTrack	   
	goodFeaturesToTrack(imageGray,corner,max_cornerNum,qualityLevel,minDistance,Mat(),blockSize,useHarrisDetector,k);
	   //画图	   
	cout << "** Number of corners detected: " << corner.size() << endl;	   
	int r = 3;  
	for (int i = 0; i < corner.size(); i++)	   
	{		   
		circle(copyImage, corner[i], r, Scalar(0,0,255),-1);
	}	   
	//显示图像   
	imshow("ST_detect", copyImage);
	imwrite("ST_05.jpg", copyImage);
}

void Fast_demo(Mat& srcImage,int threshold) {

	std::vector<KeyPoint> keypoints;
	Mat dst = srcImage.clone();
	Ptr<FastFeatureDetector> detector = FastFeatureDetector::create(threshold);
	detector->detect(src, keypoints);
	drawKeypoints(dst, keypoints, dst, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_OVER_OUTIMG);
	imshow("Fast_detect", dst);
	imwrite("Fast_05.jpg", dst);
}

void cornerHarris_demo()
{
	/// Detector parameters
	int blockSize = 2;
	int apertureSize = 3;
	double k = 0.04;
	/// Detecting corners
	Mat dst = Mat::zeros(src.size(), CV_32FC1);
	cornerHarris(src_gray, dst, blockSize, apertureSize, k);

	/// Normalizing
	Mat dst_norm, dst_norm_scaled;
	normalize(dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
	//convertScaleAbs(dst_norm, dst_norm_scaled);
	dst_norm_scaled = src.clone();

	/// Drawing a circle around corners
	for (int i = 0; i < dst_norm.rows; i++)
	{
		for (int j = 0; j < dst_norm.cols; j++)
		{
			if (thresh < 50)
				thresh = 50;
			if ((int)dst_norm.at<float>(i, j) > thresh)
			{
				circle(dst_norm_scaled, Point(j, i), 3, Scalar(0, 0, 255),-1);
			}
		}
	}
	imshow("Harris_cv", dst_norm_scaled);
	imwrite("Harris_cv05.jpg", dst_norm_scaled);
}

void myHarrisCorner_ave(Mat &srcImg) {
	Mat image,src_grayImg,Ix,Iy,I_xx,I_yy,I_xy,R,filter_R,result;
	cvtColor(srcImg, src_grayImg, COLOR_BGR2GRAY);
	image = srcImg.clone();
	
	int wsize = 3;//窗口大小

	sobelGradient(src_grayImg, Ix,1);
	
	sobelGradient(src_grayImg,Iy, 2);
	
	I_xx = computeImage(Ix, Iy, wsize, 1);
		
	I_yy = computeImage(Ix, Iy, wsize, 2);
		
	I_xy = computeImage(Ix, Iy, wsize, 4);
	
	//compute the R value
	R = computeImage(Ix, Iy, wsize, 3);
	
	filter_R = filterR(R, 10);
	

	/*imshow("ix", Ix);
	imshow("iy", Iy);
	imshow("I_xx", I_xx);
	imshow("I_yy", I_yy);
	imshow("I_xy", I_xy);
	imshow("响应图R", R);
	imshow("fr", filter_R);*/

	mixP(filter_R, image, 2);
	imshow("Ave", image);
	imwrite("myHarris_Ave05.jpg", image);
	
}

void myHarrisCorner_Gauss(Mat &srcImg) {
	Mat image, src_grayImg, Ix, Iy, I_xx, I_yy, I_xy, R, filter_R, result;
	cvtColor(srcImg, src_grayImg, COLOR_BGR2GRAY);
	image = srcImg.clone();

	int wsize = 3;//窗口大小

	sobelGradient(src_grayImg, Ix, 1);
	
	sobelGradient(src_grayImg, Iy, 2);
	
	I_xx = computeImage(Ix, Iy, wsize, 1);
	GaussianBlur(I_xx, I_xx, Size(3, 3), 0, 0);
	
	I_yy = computeImage(Ix, Iy, wsize, 2);
	GaussianBlur(I_yy, I_yy, Size(3, 3), 0, 0);
	
	I_xy = computeImage(Ix, Iy, wsize, 4);
	GaussianBlur(I_xy, I_xy, Size(3, 3), 0, 0);
	//compute the R value
	R = computeImage(I_xx, I_yy,I_xy, wsize);
	
	filter_R = filterR(R, 10);
	
	/*imshow("ix", Ix);
	imshow("iy", Iy);	
	imshow("I_xx", I_xx);	
	imshow("I_yy", I_yy);
	imshow("I_xy", I_xy);
	imshow("响应图R", R);
	imshow("fr", filter_R);*/


	mixP(filter_R, image, 2);
	imshow("Gauss", image);
	imwrite("myHarris_gauss05.jpg", image);
}

void sobelGradient(Mat& img,Mat& dst, int para) {
	dst = Mat::zeros(img.size(), CV_64FC1);
	
	if(para==1)
		Sobel(img, dst, CV_64FC1, 0, 1, 3);
	else if(para==2)
		Sobel(img, dst, CV_64FC1, 1, 0, 3);
}

Mat computeImage(Mat& ix, Mat& iy, int wsize, int para) {

	Mat I_xx, I_yy,I_xy, r;
	I_xx = Mat::zeros(ix.size(), CV_64FC1);
	I_yy = Mat::zeros(ix.size(), CV_64FC1);
	r = Mat::zeros(ix.size(), CV_64FC1);
	I_xy = Mat::zeros(ix.size(), CV_64FC1);
	

	for (int i = wsize / 2; i < (ix.rows - wsize / 2); i++)
		for (int j = wsize / 2; j < (ix.cols - wsize / 2); j++) {
			//compute A B C, A = Ix * Ix, B = Iy * Iy, C = Ix * Iy
			double A = 0;
			double B = 0;
			double C = 0;
			for (int ii = i - wsize / 2; ii <= (i + wsize / 2); ii++)
				for (int jj = j - wsize / 2; jj <= (j + wsize / 2); jj++) {
					double xx = ix.at<double>(ii, jj);
					double yy = iy.at<double>(ii, jj);
					A += xx * xx;
					B += yy * yy;
					C += xx * yy;
				}
			double p = A + B;
			double q = A * B - C * C;
			//double delta = p * p - 4 * q;//A2+B2-AB+4C2
			
			I_xx.at<double>(i, j) = A;
			I_yy.at<double>(i, j) = B;
			I_xy.at<double>(i, j) = C;
			double rr = q - 0.06 * p * p;
				
			if (rr > myHarrisThrehold_ave) {
				r.at<double>(i, j) = rr;
			}
			
		}
	switch (para) {
	case 1: return I_xx; break;
	case 2: return I_yy; break;
	case 3: return r; break;
	case 4:return I_xy; break;
	}
}

Mat filterR(Mat& img, int wsize) {
	Mat result;
	result = Mat::zeros(img.size(), CV_64F);

	//find local maxima of R
	for (int i = wsize / 2; i < (img.rows - wsize / 2); i ++)
		for (int j = wsize / 2; j < (img.cols - wsize / 2); j ++) {
			double origin = img.at<double>(i,j);
			bool found = false;
			for (int ii = i - wsize / 2; ii <= (i + wsize / 2)&&found==false; ii++)
				for (int jj = j - wsize / 2; jj <= (j + wsize / 2); jj++)
					if (origin < img.at<double>(ii, jj)) {
						origin = 0;
						found = true;
						break;
					}
			if (origin == 0) 
				result.at<double>(i, j) = 0;
			else 
				result.at<double>(i, j) = 255;
		}

	return result;
}

void mixP(Mat& point, Mat& img, int psize) {
	
	for (int i = psize; i < img.rows - psize; i++)
		for (int j = psize; j < img.cols - psize; j++) {
			if (point.at<double>(i,j) != 0) {
				for (int ii = i - psize; ii <= i + psize; ii++)
					for (int jj = j - psize; jj <= j + psize; jj++) {
						img.at<Vec3b>(ii, jj)[0] = 0;
						img.at<Vec3b>(ii, jj)[1] = 0;
						img.at<Vec3b>(ii, jj)[2] = 255;
						
					}
			}
		}
}

Mat computeImage(Mat& ixx, Mat& iyy,Mat& ixy,int wsize) {

	Mat result;	
	result = Mat::zeros(ixx.size(), CV_64FC1);


	for (int i = wsize / 2; i < (ixx.rows - wsize / 2); i++)
		for (int j = wsize / 2; j < (ixx.cols - wsize / 2); j++) {
			//compute A B C, A = Ix * Ix, B = Iy * Iy, C = Ix * Iy
			double A = 0;
			double B = 0;
			double C = 0;
			for (int ii = i - wsize / 2; ii <= (i + wsize / 2); ii++)
				for (int jj = j - wsize / 2; jj <= (j + wsize / 2); jj++) {
					double xx = ixx.at<double>(ii, jj);
					double yy = iyy.at<double>(ii, jj);
					double xy = ixy.at<double>(ii, jj);
					A += xx ;
					B += yy;
					C += xy;
				}
			double p = A + B;
			double det = A * B - C * C;
			//double delta = p * p - 4 * q;//A2+B2-AB+4C2

			double rr = det - 0.06 * p * p;

			if (rr > myHarrisThrehold_Gauss) {
				result.at<double>(i, j) = rr;
			}

		}
	return result;
}