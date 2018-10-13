#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int mainrf()
{
	cout << CV_VERSION << endl;
	//读取一张图片便显示出来
	Mat src = imread("a.png");//读取图片，要提前将图片放在程序路径下
	cvtColor(src, src, CV_BGR2GRAY);//转为灰度图
	/*
	namedWindow()函数 
	用法：namedWindow(“img”,x); 
	x=WINDOW_AUTOSIZE:用户不能手动改变窗口大小,默认为此 
	x=WINDOW_Normal:用户可以手动改变窗口大小 
	x=WINDOW_OpenGL:支持openGL
	*/
	namedWindow("img",WINDOW_NORMAL);
	imshow("img", src);
	imwrite("C:\\Users\\wangbinKF\\Desktop\\01.bmp", src);//保存结果图片
	waitKey(0);//暂停等待按键
	return 0;
}