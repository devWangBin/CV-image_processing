#include<cv.h>  
#include<highgui.h>  
#include <cmath> 
#include <stdlib.h> 
#include <iostream>
using namespace std;
const double PI = 3.141592653;

const char *strimg = "123.png";

int mainssa()
{
	IplImage * image;
	image = cvLoadImage(strimg, 1);
	cvNamedWindow("image", CV_WINDOW_AUTOSIZE);
	cvShowImage("image", image);

	/**************************************************************************************/
	//旋转图像 这里以逆时针为正  插值选用双线性插值 围绕左上角点进行旋转  双线性插值  
	IplImage* rotateimg;
	double  angle = 15;
	double Hangle = angle * PI / 180;
	int Width, Height;
	//double r,s;  
	double  r, s;

	unsigned char * ptr, *dst;

	int temp[4];
	double z1, z2;
	//原点会发生移动  
	double  fx, fy;

	double Hcos = cos(Hangle);
	double Hsin = sin(Hangle);

	//x y 存放原图中四个点的位置，以中心为原点 左上角开始 顺时针数  
	int x[4];
	int y[4];
	int x1[4];
	int y1[4];
	x[0] = -(image->width - 1) / 2;
	x[1] = -x[0];
	x[2] = -x[0];
	x[3] = x[0];
	y[0] = -(image->height - 1) / 2;
	y[1] = y[0];
	y[2] = -y[0];
	y[3] = -y[0];
	int i = 0, j = 0;
	//x1 y1 分别存放新图中图像的四个角点的位置 以中心为原点 左上角点开始 顺时针数  
	for (i = 0; i < 4; i++)
	{
		x1[i] = (int)(x[i] * Hcos - y[i] * Hsin + 0.5);
		y1[i] = (int)(x[i] * Hsin + y[i] * Hcos + 0.5);
		printf("x:   %d ", x[i]);
		printf("y:   %d ", y[i]);
		printf("x1:   %d ", x1[i]);
		printf("y1:   %d \n", y1[i]);

	}
	//确定新图像的长宽  
	if (abs(y1[2] - y1[0]) > abs(y1[3] - y1[1]))
	{
		Height = abs(y1[2] - y1[0]);
		Width = abs(x1[3] - x1[1]);
	}
	else {
		Height = abs(y1[3] - y1[1]);
		Width = abs(x1[2] - x1[0]);
	}
	rotateimg = cvCreateImage(cvSize(Width, Height), image->depth, 1);
	//两个偏移常量  
	fx = -1 * (Width - 1)* Hcos*0.5 - (Height - 1)*Hsin*0.5 + (image->width - 1) / 2;
	fy = (Width - 1)*Hsin*0.5 - (Height - 1)*Hcos*0.5 + (image->height - 1) / 2;


	for (i = 0; i < Height; i++)
	{
		for (j = 0; j < Width; j++)
		{
			dst = (unsigned char *)(rotateimg->imageData + i * rotateimg->widthStep + j);
			s = i * Hsin + j * Hcos + fx;
			r = i * Hcos - j * Hsin + fy;

			if (r < 1.0 || s < 1.0 || r >= image->height || s >= image->width)
			{
				*dst = 0;
			}
			else {
				r = r - 1;
				s = s - 1;
				ptr = (unsigned char *)(image->imageData + image->widthStep*(int)r + (int)s);
				temp[0] = *ptr;
				temp[1] = *(ptr + 1);
				ptr = (unsigned char*)(image->imageData + image->widthStep *(int)(r + 1) + int(s));
				temp[2] = *ptr;
				temp[3] = *(ptr + 1);

				z1 = (temp[1] - temp[0])*(s - int(s)) + temp[0];
				z2 = (temp[3] - temp[2])*(s - int(s)) + temp[2];
				*dst = (int)(z1 + (z2 - z1)*(r - (int)r));
			}

		}
	}
	cvNamedWindow("rotate_image", 1);
	cvShowImage("rotate_image", rotateimg);
	cvSaveImage("rotate.jpg", rotateimg);
	cvWaitKey(0);
	/**************************************************************************************/
	return 0;
}