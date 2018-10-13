#include<opencv2/core/saturate.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include<iostream>
#include<cmath>

#define PI 3.14159265 //圆周率

using namespace cv;
using namespace std;

void my_warpAffine(const Mat &src,Mat &dst,double arr[][3])
{	
	for (int x = 0; x < dst.rows; x++) {
		for (int y = 0; y < dst.cols; y++) {

		//计算在原图中的坐标（x_,y_),而后利用双线性插值采样
			double x_ = x * arr[0][0] + y * arr[0][1] + arr[0][2];
			double y_ = x * arr[1][0] + y * arr[1][1] + arr[1][2];

			if (x_ < 0 || y_ < 0||x_>=src.rows||y_>=src.cols) {
				for (int c = 0; c < 3; c++) {
					dst.at<Vec3b>(x, y)[c] = saturate_cast<uchar>(0);
				}
			}
			else {
				for (int c = 0; c < 3; c++) {
					//计算双线性插值
					//左上角坐标（X1，Y1)
					int X1 = (int)x_;
					int Y1 = (int)y_;
					//四个顶点像素值
					//注意访问越界
					if (X1 == (src.rows - 1) || Y1 == (src.cols - 1)||X1==0||Y1==0) {
						dst.at<Vec3b>(x, y)[c] = saturate_cast<uchar>(src.at<Vec3b>(X1, Y1)[c]);
					}
					else {
						int aa = src.at<Vec3b>(X1, Y1)[c];
						int bb = src.at<Vec3b>(X1, Y1+1)[c];
						int cc = src.at<Vec3b>(X1+1, Y1)[c];
						int dd = src.at<Vec3b>(X1+1, Y1+1)[c];

						double dx = x_ - X1;
						double dy = y_ - Y1;
						double h1 = aa + dx * (bb - aa);
						double h2 = cc + dx * (dd - cc);
						dst.at<Vec3b>(x, y)[c] = saturate_cast<uchar>(h1+dy*(h2-h1));
					}
					
				}
			}		
		}
	}
}

void CalcRotationMatrix(double arr[2][3],int x,int y,double angle){
	
	double Angle = angle * PI / 180;
	double s = sin(Angle);
	double c = cos(Angle);

	double t1 = (-x) * c + y * s + x;
	double t2 = (-x) * s - y * c + y;

	arr[0][0]=arr[1][1] = c;
	arr[0][1] = s;
	arr[1][0] = -s;
	arr[0][2] = -s * t2 - c * t1;
	arr[1][2] = -c * t2 + s * t1;
}

Mat Rotate(const Mat &src,int x, int y, double angle) {
	
	double arr[2][3] = { 0 };
	if (x < 0 || y < 0 || x >= src.rows || y >= src.cols) {
		cout << "please input valid rotating center!" << endl;
		exit(1);
	}
	CalcRotationMatrix(arr, x, y, angle);
	//cout << arr[0][0] << ' ' << arr[0][1] << ' ' << arr[0][2] << endl;
	//cout << arr[1][0] << ' ' << arr[1][1] << ' ' << arr[1][2] << endl;
	Mat imgAffine = Mat::zeros(src.rows, src.cols, src.type());
	my_warpAffine(src, imgAffine, arr);
	return imgAffine;
}

Mat changeShape(const Mat &src) {
	Mat imgAffine = Mat::zeros(src.rows, src.cols, src.type());	
	int row = imgAffine.rows, col = imgAffine.cols;
	for (int x = 0; x < row; x++) {
		for (int y = 0; y < col; y++) {
			
			double X = x / ((row - 1) / 2.0) - 1.0;
			double Y = y / ((col - 1) / 2.0) - 1.0;
			double r = sqrt(X * X + Y * Y);
			
			if (r >= 1) {
				imgAffine.at<Vec3b>(x, y)[0] = saturate_cast<uchar>(src.at<Vec3b>(x, y)[0]);
				imgAffine.at<Vec3b>(x, y)[1] = saturate_cast<uchar>(src.at<Vec3b>(x, y)[1]);
				imgAffine.at<Vec3b>(x, y)[2] = saturate_cast<uchar>(src.at<Vec3b>(x, y)[2]);
			}
			else {

				double theta = 1.0 + X * X + Y * Y - 2.0*sqrt(X * X + Y * Y);//修改不用（1-r)*(1-r)
				double x_ = cos(theta)*X - sin(theta)*Y;
				double y_ = sin(theta)*X + cos(theta)*Y;

				x_ = (x_ + 1.0)*((row - 1) / 2.0);
				y_ = (y_ + 1.0)*((col - 1) / 2.0);

				
				if (x_ < 0 || y_ < 0||x_>=src.rows||y_>=src.cols) {
					for (int c = 0; c < 3; c++) {
						imgAffine.at<Vec3b>(x, y)[c] = saturate_cast<uchar>(0);
					}
				}
				else {
					//左上角坐标（X1，Y1)
					//计算双线性插值	
					int X1 = (int)x_;
					int Y1 = (int)y_;

					for (int c = 0; c < 3; c++) {	
						if (X1 == (src.rows - 1) || Y1 == (src.cols - 1)) {
							imgAffine.at<Vec3b>(x, y)[c] = saturate_cast<uchar>(src.at<Vec3b>(X1, Y1)[c]);
						}
						else {
							//四个顶点像素值
							//注意访问越界
							int aa = src.at<Vec3b>(X1, Y1)[c];
							int bb = src.at<Vec3b>(X1, Y1 + 1)[c];
							int cc = src.at<Vec3b>(X1 + 1, Y1)[c];
							int dd = src.at<Vec3b>(X1 + 1, Y1 + 1)[c];

							double dx = x_ - (double)X1;
							double dy = y_ - (double)Y1;
							double h1 = aa + dx * (bb - aa);
							double h2 = cc + dx * (dd - cc);
							imgAffine.at<Vec3b>(x, y)[c] = saturate_cast<uchar>(h1 + dy * (h2 - h1));
						}
					}
				}
			}	
		}
	}
	return imgAffine;
}

int main() {

	Mat imgAffine,imgChange, image;
	image = imread("1234.png", 1);

	cout << "图像仿射变换程序开始...\n请输入旋转中心（x,y）>>>" << endl;
	int x=0, y=0;
	cin >> x >> y;
	cout << "请输入偏转角度>>>";
	double angle=45.0;
	cin >> angle;

	imgAffine = Rotate(image,x, y,angle);
	imgChange = changeShape(image);

	
	namedWindow("旋转后图片", WINDOW_AUTOSIZE);
	imshow("旋转后图片", imgAffine);

	namedWindow("变形后图片", WINDOW_AUTOSIZE);
	imshow("变形后图片", imgChange);
	waitKey(0);
	//system("pause");
	return 0;

}