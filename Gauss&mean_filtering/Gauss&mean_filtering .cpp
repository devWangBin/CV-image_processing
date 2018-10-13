#include<opencv2/core/saturate.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include<iostream>
#include<cmath>
using namespace std;
using namespace cv;


void MyGaussianFilter(int pos, void* usrdata)
{ 
	const Mat SRC = *(Mat*)(usrdata);
	Mat src;
	SRC.copyTo(src);
	double sigma = pos * 0.01;
	Mat dst = Mat::zeros(src.rows, src.cols, src.type());

	int ksize = sigma * 6 - 1;
	if (ksize < 3) {
		cout << "ksize is too small!error" << endl;
		return;
	}
	int iRes = ksize % 2;
	if (iRes == 0) {
		ksize += 1;
	}
	
	CV_Assert(src.channels() == 1 || src.channels() == 3); // 只处理单通道或者三通道图像
	// 生成一维的高斯滤波模板核
	double *GS_filter = new double[ksize];
	//得到前面的系数分母用于归一化
	double sum = 0;
	int half = ksize / 2;
	//初始化滤波核
	for (int i = 0; i < ksize; i++)
	{
		// 只需计算指数部分，高斯函数前的常数可以不用计算，会在归一化的过程中给消去
		//以（half,half）为中心建立坐标系进行计算
		double g = exp(-(i - half) * (i - half) / (2 * sigma * sigma));
		sum += g;
		GS_filter[i] = g;
	}
	// 归一化
	for (int i = 0; i < ksize; i++)
		GS_filter[i] /= sum;

	// 将模板应用到图像中
	int border = ksize / 2;//ksize如果不是奇数的话可能会出bug
	//边缘处理：扩充边缘
	//BORDER_REFLECT_101：对称法，也就是以最边缘像素为轴对称进行扩充
	copyMakeBorder(src, dst, border, border, border, border, BorderTypes::BORDER_CONSTANT);//BORDER_REFLECT_101BORDER_REPLICATE

	int channels = dst.channels();
	int rows = dst.rows - border;
	int cols = dst.cols - border;
	// 水平方向
	for (int i = border; i < rows; i++)
	{
		for (int j = border; j < cols; j++)
		{
			double sum[3] = { 0 };
			for (int k = -border; k <= border; k++)
			{
				//对单通道和三通道图像分别处理
				if (channels == 1)
				{
					sum[0] += GS_filter[border + k] * dst.at<uchar>(i, j + k); // 行不变，列变化；先做水平方向的卷积
				}
				else if (channels == 3)
				{
					Vec3b rgb = dst.at<Vec3b>(i, j + k);
					sum[0] += GS_filter[border + k] * rgb[0];
					sum[1] += GS_filter[border + k] * rgb[1];
					sum[2] += GS_filter[border + k] * rgb[2];
				}
			}
			//堆像素进行规整处理避免溢出
			for (int k = 0; k < channels; k++)
			{
				if (sum[k] < 0)
					sum[k] = 0;
				else if (sum[k] > 255)
					sum[k] = 255;
			}
			if (channels == 1)
				src.at<uchar>(i- border, j- border) = static_cast<uchar>(sum[0]);
			else if (channels == 3)
			{
				Vec3b rgb = { static_cast<uchar>(sum[0]), static_cast<uchar>(sum[1]), static_cast<uchar>(sum[2]) };
				src.at<Vec3b>(i- border, j- border) = rgb;
			}
		}
	}
	// 竖直方向
	for (int i = border; i < rows; i++)
	{
		for (int j = border; j < cols; j++)
		{
			double sum[3] = { 0 };
			for (int k = -border; k <= border; k++)
			{
				if (channels == 1)
				{
					sum[0] += GS_filter[border + k] * dst.at<uchar>(i + k, j); // 列不变，行变化；竖直方向的卷积
				}
				else if (channels == 3)
				{
					Vec3b rgb = dst.at<Vec3b>(i + k, j);
					sum[0] += GS_filter[border + k] * rgb[0];
					sum[1] += GS_filter[border + k] * rgb[1];
					sum[2] += GS_filter[border + k] * rgb[2];
				}
			}
			for (int k = 0; k < channels; k++)
			{
				if (sum[k] < 0)
					sum[k] = 0;
				else if (sum[k] > 255)
					sum[k] = 255;
			}
			if (channels == 1)
				src.at<uchar>(i- border, j- border) = static_cast<uchar>(sum[0]);
			else if (channels == 3)
			{
				Vec3b rgb = { static_cast<uchar>(sum[0]), static_cast<uchar>(sum[1]), static_cast<uchar>(sum[2]) };
				src.at<Vec3b>(i- border, j- border) = rgb;
			}
		}
	}
	delete[] GS_filter;
	imshow("高斯平滑图片", src);
}

void MyIntegralImage(const Mat &SRC,int width,int height, int *Integral) {

	Mat src;
	SRC.copyTo(src);
	
	int channels = src.channels();

	if (channels == 1) {
		cout << "1通道图片均值滤波..." << endl;
		int *prow = new int[width];
		memset(prow, 0, sizeof(int)*(width));

		for (int s = 0; s < width + 1; s++) {
			Integral[s] = 0;
		}
		Integral += (width + 1);
		for (int yi = 1; yi < height+1; ++yi,Integral += (width+1)) {
		
			//对第一列像素值单独处理	
			
			Integral[0] = 0;		
			prow[0] += src.at<uchar>(yi - 1, 0);
			
			Integral[1] = prow[0];
			
			for (int xi = 2; xi < width+1; ++xi)
			{
				prow[xi-1] += src.at<uchar>(yi-1,xi-1);
				Integral[xi] = Integral[xi - 1] + prow[xi-1];
			}
		}
		delete[]prow;
	}
	else if (channels == 3) {
		cout << "3通道图片均值滤波..." << endl;

		int *prow = new int[width*3];
		memset(prow, 0, sizeof(int)*width*3);

		for (int s = 0; s < 3*(width + 1); s++) {
			Integral[s] = 0;
		}
		Integral += 3*(width + 1);
		
		for (int yi = 1; yi < height+1; ++yi, Integral += 3 * (width + 1)) {

			//对第一列像素值单独处理
			Integral[0] = 0;
			Integral[1] = 0;
			Integral[2] = 0;

			Vec3b rgb = src.at<Vec3b>(yi-1, 0);
			prow[0] += rgb[0];
			prow[1] += rgb[1];
			prow[2] += rgb[2];
		
			Integral[3] = prow[0];
			Integral[4] = prow[1];
			Integral[5] = prow[2];
			
			for (int xi = 2; xi < width+1; ++xi)
			{
				rgb = src.at<Vec3b>(yi-1, xi-1);
				prow[3*(xi-1)+0] += rgb[0];
				prow[3*(xi-1)+1] += rgb[1];
				prow[3*(xi-1)+2] += rgb[2];

				Integral[3 * xi + 0] = Integral[3 * (xi - 1) + 0] + prow[3 * (xi-1) + 0];
				Integral[3 * xi + 1] = Integral[3 * (xi - 1) + 1] + prow[3 * (xi-1) + 1];
				Integral[3 * xi + 2] = Integral[3 * (xi - 1) + 2] + prow[3 * (xi-1) + 2];
			}
		}		
		delete[]prow;
	}	
}

void MyMeanFliter(int w_size, void* usrdata) {

	if (w_size < 3) {
		cout << "ksize is too small!error" << endl;
		return;
	}
	int iRes = w_size % 2;
	if (iRes == 0) {
		w_size += 1;
	}

	const Mat SRC = *(Mat*)(usrdata);
	Mat src;
	SRC.copyTo(src);
	int border = w_size / 2;//ksize如果不是奇数的话可能会出bug
	//边缘处理：扩充边缘
	//BORDER_REFLECT_101：对称法，也就是以最边缘像素为轴对称进行扩充
	copyMakeBorder(SRC, src, border, border, border, border, BorderTypes::BORDER_REFLECT_101);//BORDER_REFLECT_101BORDER_REPLICATEBORDER_CONSTANT

	int width = src.cols;
	int height = src.rows;

	cout << width << ' ' << height << ' ' << endl;

	//用于存储积分图
	int *Integral;
	int channels = src.channels();
	//目标图像
	Mat dst = Mat::zeros(SRC.size(), SRC.type());

	if (channels == 1) {
		cout << "单通道图片均值滤波..." << endl;
		Integral = new int[(width+1)*(height+1)];
		MyIntegralImage(src, width,height,Integral);
		int *p = Integral;
	
		int guiyi = (2 * border + 1)*(2 * border + 1);

		for (int yi = 0; yi < dst.rows; ++yi) {
			for (int xi = 0; xi < dst.cols; ++xi) {
				/*int arr[4][2] = { 0 };
				arr[0][0] = yi;
				arr[0][1] = xi;

				arr[1][0] = yi + border + border+1;
				arr[1][1] = xi;

				arr[2][0] = yi;
				arr[2][1] = xi + border + border+1;

				arr[3][0] = arr[1][0];
				arr[3][1] = arr[2][1];*/

				int s1 = *(Integral + yi * (width + 1) + xi);
				int s2 = *(Integral + (yi + border + border + 1)*(width + 1) + xi);
				int s3 = *(Integral + yi * (width + 1) + xi + border + border + 1);
				int s4 = *(Integral + (yi + border + border + 1)*(width + 1) + (xi + border + border + 1));
				
				int gole = (s4 + s1 - s2 - s3) / guiyi;

				if (gole < 0)
					gole = 0;
				else if(gole > 255)
					gole = 255;

				dst.at<uchar>(yi,xi) = static_cast<uchar>(gole);
			}
		}
	}
	else if (channels == 3) {

		cout << "三通道图片均值滤波..." << endl;
		Integral = new int[(width + 1)*(height + 1) * 3];
		MyIntegralImage(src, width, height,Integral);

		int guiyi = (2 * border + 1)*(2 * border + 1);


		for (int yi = 0; yi < dst.rows; ++yi) {
			for (int xi = 0; xi < dst.cols; ++xi) {
				int s11 = *(Integral + yi * 3 * (width + 1) + xi * 3);
				int s12 = *(Integral + yi * 3 * (width + 1) + xi * 3 + 1);
				int s13 = *(Integral + yi * 3 * (width + 1) + xi * 3 + 2);

				int s21 = *(Integral + (yi + border + border + 1) * 3 * (width + 1) + xi * 3);
				int s22 = *(Integral + (yi + border + border + 1) * 3 * (width + 1) + xi * 3 + 1);
				int s23 = *(Integral + (yi + border + border + 1) * 3 * (width + 1) + xi * 3 + 2);

				int s31 = *(Integral + yi * 3 * (width + 1) + (xi + border + border + 1) * 3);
				int s32 = *(Integral + yi * 3 * (width + 1) + (xi + border + border + 1) * 3 + 1);
				int s33 = *(Integral + yi * 3 * (width + 1) + (xi + border + border + 1) * 3 + 2);

				int s41 = *(Integral + (yi + border + border + 1) * 3 * (width + 1) + (xi + border + border + 1) * 3);
				int s42 = *(Integral + (yi + border + border + 1) * 3 * (width + 1) + (xi + border + border + 1) * 3 + 1);
				int s43 = *(Integral + (yi + border + border + 1) * 3 * (width + 1) + (xi + border + border + 1) * 3 + 2);

				int gole[3];
				 gole[0] = (s41 + s11 - s21 - s31) / guiyi;
				 gole[1] = (s42 + s12 - s22 - s32) / guiyi;
				 gole[2] = (s43 + s13 - s23 - s33) / guiyi;
				for (int i = 0; i < 3; i++) {
					if (gole[i] < 0)
						gole[i] = 0;
					else if (gole[i] > 255)
						gole[i] = 255;
				}		
				Vec3b rgb = { static_cast<uchar>(gole[0]), static_cast<uchar>(gole[1]), static_cast<uchar>(gole[2]) };
				dst.at<Vec3b>(yi, xi) = rgb;			
			}
		}
	}
	imshow("均值滤波图片", dst);
}


int main() {
	
	Mat image;
	image = imread("1234.png",0);
	if (!image.data)
	{
		cout << "图片读取失败！" << endl;
		return -1;
	}	
	cout << "图像高斯平滑程序开始..." << endl;
	namedWindow("原图", WINDOW_AUTOSIZE);
	imshow("原图", image);
	cout << image.cols << ' ' << image.rows << ' ' << endl;

	int sigma = 80;
	namedWindow("高斯平滑图片", WINDOW_AUTOSIZE);
	createTrackbar("sigma*0.01", "高斯平滑图片", &sigma, 500, MyGaussianFilter,&image);
	MyGaussianFilter(80, &image);

	int win_size = 3;
	namedWindow("均值滤波图片", WINDOW_AUTOSIZE);
	createTrackbar("window_size", "均值滤波图片", &win_size, 30, MyMeanFliter, &image);
	MyMeanFliter(3, &image);
	
	waitKey(0);
	return 0;
}