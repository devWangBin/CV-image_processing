#include <iostream>
#include <core/core.hpp>  
#include<opencv2/core/saturate.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include<cmath>
using namespace std;
using namespace cv;

Mat image; 
Mat imageCopy; //绘制矩形框时用来拷贝原图的图像
Mat targetImage;//截取的目标图像
bool leftButtonDownFlag=false; //左键单击后视频暂停播放的标志位
Point originalPoint; //矩形框起点
Point processPoint; //矩形框终点
const char* videoPath = "c505.mp4";

//将一部分变量设为全局函数，提高运行效率
int cccc = 0;//判断目标标记完成
//H、S通道
int channels[] = { 0, 1 };
int histSize[] = { 30, 32 };
float HRanges[] = { 0, 180 };
float SRanges[] = { 0, 256 };
const float *ranges[] = { HRanges, SRanges };

void onMouse(int event, int x, int y, int flags, void* ustc); //鼠标回调函数
void getTarget();
void drawHist(Mat rectImage);
void tracing(const MatND srcHist);
double compHist(const MatND srcHist, Mat compareImage);

int main()
{
	getTarget();
	imshow("Target object", targetImage);
	cout << originalPoint << processPoint << endl;
	drawHist(targetImage);
	Mat srcHsvImage;
	cvtColor(targetImage, srcHsvImage, CV_BGR2HSV);
	//采用H-S直方图进行处理
	//首先得配置直方图的参数
	MatND srcHist;
	//进行原图直方图的计算
	calcHist(&srcHsvImage, 1, channels, Mat(), srcHist, 2, histSize, ranges, true, false);
	//归一化
	normalize(srcHist, srcHist, 0, 1, NORM_MINMAX);
	
	tracing(srcHist);
	waitKey(0);
	return 0;
}

void tracing(const MatND srcHist) {
	int width = abs(processPoint.x - originalPoint.x);
	int height = abs(processPoint.y - originalPoint.y);
	cout << width << height << endl;
	//目标搜索区域设定为原区域的周围且面积为原来三倍
	int X1 = originalPoint.x - width;
	int X2 = originalPoint.x + width;
	int Y1 = originalPoint.y - height;
	int Y2 = originalPoint.y + height;
	//越界检查
	if (X1 < 0)
		X1 = 0;
	if (Y1 < 0)
		Y1 = 0;

	Point preStart;
	Point preEnd;

	Point get1(0, 0);
	Point get2(0, 0);
	
	VideoCapture video(videoPath);
	if (!video.isOpened())
	{
		cout << "video not open.error!" << std::endl;
		return;
	}
	double fps = video.get(CV_CAP_PROP_FPS); //获取视频帧率
	double pauseTime = 1000 / fps; //两幅画面中间间隔
	int w = static_cast<int>(video.get(CV_CAP_PROP_FRAME_WIDTH));
	int h = static_cast<int>(video.get(CV_CAP_PROP_FRAME_HEIGHT));
	cv::Size S(w, h);
	namedWindow("targetTracing");
	
	VideoWriter write;
	write.open("c501_test.avi", -1, fps, S, true);
	while (true)
	{
		//取视频的一帧
		video >> image;
		if (!image.data || waitKey(pauseTime) == 27)  //图像为空或Esc键按下退出播放
		{
			break;
		}
		//直方图对比的相似值
		double comnum = 1.0;
		//初始化搜索区域
		for (int Cy = Y1; Cy <= Y2; Cy += 10) {
			for (preStart.x = X1, preStart.y = Cy; preStart.x <= X2; preStart.x += 10) {

				if ((preStart.x + width) < image.cols)
					preEnd.x = preStart.x + width;
				else
					preEnd.x = image.cols - 1;
				if ((preStart.y + height) < image.rows)
					preEnd.y = preStart.y + height;
				else
					preEnd.y = image.rows - 1;
				//cout << preStart << preEnd << endl;

				Mat compareImg;
				compareImg = image(Rect(preStart, preEnd));
				double c = compHist(srcHist, compareImg);
				if (comnum > c) {
					get1 = preStart;
					get2 = preEnd;
					comnum = c;
				}
			}
		}
		//在原始视频图像上刷新矩形，只有当与目标直方图很相似时才更新起点搜索区域，满足目标进行移动的场景
		if (comnum < 0.15) {
			X1 = get1.x - width;
			X2 = get1.x + width;
			Y1 = get1.y - height;
			Y2 = get1.y + height;

			if (X1 < 0)
				X1 = 0;
			if (Y1 < 0)
				Y1 = 0;
			
		}
		if(comnum<0.5)
			rectangle(image, get1, get2, Scalar(0, 0, 255), 2);
		//写入一帧
		write.write(image);
		imshow("targetTracing", image);
	}
	video.release();
	write.release();
}

//画出直方图
void drawHist(const Mat rectImage)
{
	//定义函数需要的一些变量
	//图片数量nimages
	int nimages = 1;
	//通道数量,我们总是习惯用数组来表示，后面会讲原因
	int channels[3] = { 0,1,2 };
	//输出直方图
	cv::Mat outputHist_red, outputHist_green, outputHist_blue;
	//维数
	int dims = 1;
	//存放每个维度直方图尺寸（bin数量）的数组histSize
	int histSize[3] = { 256,256,256 };
	//每一维数值的取值范围ranges
	float hranges[2] = { 0, 255 };
	//值范围的指针
	const float *ranges[3] = { hranges,hranges,hranges };
	//是否均匀
	bool uni = true;
	//是否累积
	bool accum = false;

	//计算图像的直方图(红色通道部分)
	cv::calcHist(&rectImage, nimages, &channels[0], cv::Mat(), outputHist_red, dims, &histSize[0], &ranges[0], uni, accum);
	//计算图像的直方图(绿色通道部分)
	cv::calcHist(&rectImage, nimages, &channels[1], cv::Mat(), outputHist_green, dims, &histSize[1], &ranges[1], uni, accum);
	//计算图像的直方图(蓝色通道部分)
	cv::calcHist(&rectImage, nimages, &channels[2], cv::Mat(), outputHist_blue, dims, &histSize[2], &ranges[2], uni, accum);

	//遍历每个箱子(bin)检验，这里的是在控制台输出的。
	//for (int i = 0; i < 256; i++)
		//std::cout << "bin/value:" << i << "=" << outputHist_red.at<float>(i) << std::endl;

	//画出直方图
	int scale = 1;
	//直方图的图片,因为尺寸是一样大的,所以就以histSize[0]来表示全部了.
	cv::Mat histPic(histSize[0], histSize[0] * scale * 3, CV_8UC3, cv::Scalar(0, 0, 0));
	//找到最大值和最小值,索引从0到2分别是红,绿,蓝
	double maxValue[3] = { 0, 0, 0 };
	double minValue[3] = { 0, 0, 0 };
	cv::minMaxLoc(outputHist_red, &minValue[0], &maxValue[0], NULL, NULL);
	cv::minMaxLoc(outputHist_green, &minValue[1], &maxValue[1], NULL, NULL);
	cv::minMaxLoc(outputHist_blue, &minValue[2], &maxValue[2], NULL, NULL);
	//测试
	std::cout << minValue[0] << " " << minValue[1] << " " << minValue[2] << std::endl;
	std::cout << maxValue[0] << " " << maxValue[1] << " " << maxValue[2] << std::endl;

	//纵坐标缩放比例
	double rate_red = (histSize[0] / maxValue[0])*0.9;
	double rate_green = (histSize[0] / maxValue[1])*0.9;
	double rate_blue = (histSize[0] / maxValue[2])*0.9;

	for (int i = 0; i < histSize[0]; i++)
	{
		float value_red = outputHist_red.at<float>(i);
		float value_green = outputHist_green.at<float>(i);
		float value_blue = outputHist_blue.at<float>(i);
		//分别画出直线
		cv::line(histPic, cv::Point(i*scale, histSize[0]), cv::Point(i*scale, histSize[0] - value_red * rate_red), cv::Scalar(0, 0, 255));
		cv::line(histPic, cv::Point((i + 256)*scale, histSize[0]), cv::Point((i + 256)*scale, histSize[0] - value_green * rate_green), cv::Scalar(0, 255, 0));
		cv::line(histPic, cv::Point((i + 512)*scale, histSize[0]), cv::Point((i + 512)*scale, histSize[0] - value_blue * rate_blue), cv::Scalar(255, 0, 0));
	}
	cv::imshow("histgram", histPic);
	
}

//启动函数加载视频，并交互框出目标图像
void getTarget() {
	VideoCapture video(videoPath);
	if (!video.isOpened())
	{
		cout << "video not open.error!" << std::endl;
		return;
	}
	double fps = video.get(CV_CAP_PROP_FPS); //获取视频帧率
	double pauseTime = 1000 / fps; //两幅画面中间间隔
	namedWindow("Friends");
	setMouseCallback("Friends", onMouse);
	while (true)
	{
		if (!leftButtonDownFlag) //判定鼠标左键没有按下，采取播放视频，否则暂停
		{
			video >> image;
		}
		if (!image.data || waitKey(pauseTime) == 27)  //图像为空或Esc键按下退出播放
		{
			break;
		}
		//两种情况下不在原始视频图像上刷新矩形
		//1. 起点等于终点
		//2. 左键按下且未抬起
		if (originalPoint != processPoint && !leftButtonDownFlag)
		{
			rectangle(image, originalPoint, processPoint, Scalar(255, 0, 0), 2);
		}
		imshow("Friends", image);
		if (cccc == 1) {
			destroyWindow("Friends");
			break;
		}
			
	}
	video.release();
}

//鼠标回调函数  
void onMouse(int event, int x, int y, int flags, void *ustc)
{
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		leftButtonDownFlag = true; //标志位
		originalPoint = Point(x, y);  //设置左键按下点的矩形起点
		processPoint = originalPoint;
	}
	if (event == CV_EVENT_MOUSEMOVE && leftButtonDownFlag)
	{
		imageCopy = image.clone();
		processPoint = Point(x, y);

		if (originalPoint != processPoint)
		{
			//在复制的图像上绘制矩形
			rectangle(imageCopy, originalPoint, processPoint, Scalar(255, 0, 0), 2);
		}
		imshow("Friends", imageCopy);
		
	}
	if (event == CV_EVENT_LBUTTONUP)
	{
		leftButtonDownFlag = false;
		targetImage = image(Rect(originalPoint, processPoint)); //获取目标图像targetImage
		cccc = 1;
	}
}

double compHist(const MatND srcHist,Mat compareImage)
{
	//在比较直方图时，最佳操作是在HSV空间中操作，所以需要将BGR空间转换为HSV空间
	Mat compareHsvImage;
	cvtColor(compareImage, compareHsvImage, CV_BGR2HSV);
	//采用H-S直方图进行处理
	//首先得配置直方图的参数
	MatND  compHist;
	//进行原图直方图的计算
	
	//对需要比较的图进行直方图的计算
	calcHist(&compareHsvImage, 1, channels, Mat(), compHist, 2, histSize, ranges, true, false);
	//注意：这里需要对两个直方图进行归一化操作
	
	normalize(compHist, compHist, 0, 1, NORM_MINMAX);
	//对得到的直方图对比
	double g_dCompareRecult = compareHist(srcHist, compHist, 3);//3表示采用巴氏距离进行两个直方图的比较
	return g_dCompareRecult;
}
