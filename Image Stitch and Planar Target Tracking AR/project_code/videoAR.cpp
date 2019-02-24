#include <opencv2/opencv.hpp>
#include <iostream>
#include<opencv2/xfeatures2d.hpp>
#include<opencv2/features2d/features2d.hpp>
#include<cmath>

using namespace cv;
using namespace std;

const char* videoPath = "video_pro.mp4";
const char* videoPath02 = "video03.mp4";
Mat image999;
Mat imageCopy999; //绘制矩形框时用来拷贝原图的图像
bool leftButtonDownFlag = false; //左键单击后视频暂停播放的标志位
int cccc = 0;//判断目标标记完成
//主要得到一个矩形区域，根据该区域来计算特征点求homograph变换
Point originalPoint; //矩形框起点
Point processPoint; //矩形框终点

struct userdata {
	Mat im;
	vector<Point2f> points;
};

void mouseHandler(int event, int x, int y, int flags, void* data_ptr)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		userdata *data = ((userdata *)data_ptr);
		circle(data->im, Point(x, y), 3, Scalar(0, 255, 255), 5, CV_AA);
		imshow("Image", data->im);
		if (data->points.size() < 4)
		{
			data->points.push_back(Point2f(x, y));
		}
	}

}

Mat getmyHomographMat(Mat &lastPart, Mat &curPart) {
	//灰度图转换  
	Mat image1, image2;
	cvtColor(lastPart, image1, CV_RGB2GRAY);
	cvtColor(curPart, image2, CV_RGB2GRAY);

	//提取特征点
	Ptr<xfeatures2d::SURF> detector = xfeatures2d::SURF::create(2000);

	vector<KeyPoint> keyPoint1, keyPoint2;
	Mat imageDesc1, imageDesc2;
	detector->detectAndCompute(image1, Mat(), keyPoint1, imageDesc1);
	detector->detectAndCompute(image2, Mat(), keyPoint2, imageDesc2);

	FlannBasedMatcher matcher;
	vector<vector<DMatch> > matchePoints;
	vector<DMatch> GoodMatchePoints;

	set< pair<int, int>>  matches;

	vector<Mat> train_desc(1, imageDesc1);
	matcher.add(train_desc);
	matcher.train();

	matcher.knnMatch(imageDesc2, matchePoints, 2);
	//cout << "全部匹配点数目：" << matchePoints.size() << endl;

	// Lowe's algorithm,获取优秀匹配点
	for (int i = 0; i < matchePoints.size(); i++)
	{
		if (matchePoints[i][0].distance < 0.5 * matchePoints[i][1].distance)
		{
			GoodMatchePoints.push_back(matchePoints[i][0]);
			matches.insert(make_pair(matchePoints[i][0].queryIdx, matchePoints[i][0].trainIdx));
		}
	}
	//cout << "1->2 good 匹配点数目：" << GoodMatchePoints.size() << endl;

	//可选是否进行二次特征点匹配筛选
#if 1

	FlannBasedMatcher matcher2;
	matchePoints.clear();
	vector<Mat> train_desc2(1, imageDesc2);
	matcher2.add(train_desc2);
	matcher2.train();

	matcher2.knnMatch(imageDesc1, matchePoints, 2);
	// Lowe's algorithm,获取优秀匹配点
	for (int i = 0; i < matchePoints.size(); i++)
	{
		if (matchePoints[i][0].distance < 0.5 * matchePoints[i][1].distance)
		{
			if (matches.find(make_pair(matchePoints[i][0].trainIdx, matchePoints[i][0].queryIdx)) == matches.end())
			{
				GoodMatchePoints.push_back(DMatch(matchePoints[i][0].trainIdx, matchePoints[i][0].queryIdx, matchePoints[i][0].distance));
			}

		}
	}
	cout << "1->2 & 2->1 good 匹配点数目：" << GoodMatchePoints.size() << endl;
#endif

	vector<Point2f> imagePoints1, imagePoints2;

	for (int i = 0; i < GoodMatchePoints.size(); i++)
	{
		imagePoints2.push_back(keyPoint2[GoodMatchePoints[i].queryIdx].pt);
		imagePoints1.push_back(keyPoint1[GoodMatchePoints[i].trainIdx].pt);
	}
	Mat homo = findHomography(imagePoints1, imagePoints2,CV_RANSAC);
	return homo;
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
		imageCopy999 = image999.clone();
		processPoint = Point(x, y);

		if (originalPoint != processPoint)
		{
			//在复制的图像上绘制矩形
			rectangle(imageCopy999, originalPoint, processPoint, Scalar(0, 0, 255), 2);
		}
		
		imshow("Friends", imageCopy999);

	}
	if (event == CV_EVENT_LBUTTONUP)
	{
		leftButtonDownFlag = false;
		Mat targetImage999 = image999(Rect(originalPoint, processPoint)); //子图像显示j
		imshow("zsds", targetImage999);
		waitKey(0);
		cccc = 1;
	}
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
			video >> image999;
		}
		if (!image999.data || waitKey(pauseTime) == 27)  //图像为空或Esc键按下退出播放
		{
			break;
		}
		//两种情况下不在原始视频图像上刷新矩形
		//1. 起点等于终点
		//2. 左键按下且未抬起
		if (originalPoint != processPoint && !leftButtonDownFlag)
		{
			rectangle(image999, originalPoint, processPoint, Scalar(255, 0, 0), 2);
		}
		//rectangle(image999, originalPoint, processPoint, Scalar(255, 0, 0), 2);
		imshow("Friends", image999);
		if (cccc == 1) {
			destroyWindow("Friends");
			break;
		}
	}
	video.release();
}

int main() {

	getTarget();
	//waitKey(0);
	cout << "矩形框起点：" << originalPoint << endl;
	cout << "矩形框终点：" << processPoint << endl;
	destroyAllWindows();

	cout << "start!!!" << endl;
	VideoCapture video(videoPath);
	VideoCapture myvideo(videoPath02);

	if ((!video.isOpened())||(!myvideo.isOpened()))
	{
		cout << "video open error!" << endl;
		return -1;
	}
	double fps = video.get(CV_CAP_PROP_FPS); //获取视频帧率
	cout << "video fps:" << fps << endl;
	double pauseTime = 1000 / fps; //两幅画面中间间隔
	//long num_frame = static_cast<long>(video.get(CV_CAP_PROP_FRAME_COUNT));
	//int update_num = num_frame / 20;
	int WY = static_cast<int>(video.get(CV_CAP_PROP_FRAME_WIDTH));
	int HY = static_cast<int>(video.get(CV_CAP_PROP_FRAME_HEIGHT));
	cv::Size S(WY, HY);

	Mat FstImg, shImg, vimg;
	video >> FstImg;
	vimg = FstImg.clone();
	// Read in the image.
	Mat inImg;
	myvideo >> inImg;
	Size size = inImg.size();
	
	// Create a vector of points.
	vector<Point2f> pts_src;
	pts_src.push_back(Point2f(0, 0));
	pts_src.push_back(Point2f(size.width - 1, 0));
	pts_src.push_back(Point2f(size.width - 1, size.height - 1));
	pts_src.push_back(Point2f(0, size.height - 1));

	// Set data for mouse handler
	Mat im_temp = FstImg.clone();
	userdata data;
	data.im = im_temp;

	//show the image
	imshow("Image", im_temp);

	cout << "Click on four corners of a billboard and then press ENTER" << endl;
	//set the callback function for any mouse event
	setMouseCallback("Image", mouseHandler, &data);
	waitKey(0);

	cout << "size data point :"<<data.points.size() << endl;

	// Calculate Homography between source and destination points
	Mat h = findHomography(pts_src, data.points);

	// Warp source image
	warpPerspective(inImg, im_temp, h, im_temp.size());

	// Extract four points from mouse data
	Point pts_dst[4];
	for (int i = 0; i < 4; i++)
	{
		pts_dst[i] = data.points[i];
	}
	// Black out polygonal area in destination image.
	fillConvexPoly(vimg, pts_dst, 4, Scalar(0), CV_AA);

	// Add warped source image to destination image.
	shImg = vimg + im_temp;
	Mat curImg;

	vector<Point2f> CURponits;
	//Mat lastPart = FstImg(Rect(originalPoint, processPoint));//提升速度

	VideoWriter writerr;
	writerr.open("video_out_final.mp4", CV_FOURCC('D', 'I', 'V', 'X'), fps, S, true);
	//imshow("result", shImg);
	writerr.write(shImg);
	int count_frame = 1;
	//Mat lastPart = FstImg(Rect(originalPoint, processPoint));//提升速度
	while (true) {
		video >> curImg; myvideo >> inImg;		
		if (!inImg.data||!curImg.data || waitKey(pauseTime) == 27)  //图像为空或Esc键按下退出播放
		{
			break;
		}
		im_temp = curImg.clone();
		vimg = curImg.clone();		
		//Mat curPart = curImg(Rect(originalPoint, processPoint));
		//h = getmyHomographMat(lastPart, curPart);
		h = getmyHomographMat(FstImg, curImg);
		//cout << "变换矩阵为：\n" << h << endl << endl; //输出映射矩阵		
		perspectiveTransform(data.points,CURponits, h);
		//cout << "size data point :" << data.points.size() << endl;
		Mat HH = findHomography(pts_src, CURponits);
		warpPerspective(inImg, inImg, HH, curImg.size());
		for (int i = 0; i < 4; i++)
		{
			pts_dst[i] = CURponits[i];
		}		
		fillConvexPoly(vimg, pts_dst, 4, Scalar(0), CV_AA);
		shImg = vimg + inImg;
		//不去更新对比图片，如果每次在上一次的点基础上变换，会导致误差不断累积
		//imshow("result", shImg);
		count_frame++;
		if (count_frame % 10 == 0) {
			FstImg = im_temp;
			//lastPart = im_temp(Rect(originalPoint, processPoint));//提升速度
			data.points = CURponits;
		}
		writerr.write(shImg);
		//shImg.release();		
	}
	//waitKey(0);
	writerr.release();
	video.release();
	myvideo.release();
	return 0;
}



int main()
{
	//VideoCapture capture;
	//capture.open("video_pro.mp4");//VideoCapture类的方法
	//long num_frame = static_cast<long>(capture.get(CV_CAP_PROP_FRAME_COUNT));
	//cout << num_frame << endl;
	//capture.release();
	// Read in the image.
	Mat im_src = imread("target.jpg");
	Size size = im_src.size();

	// Create a vector of points.
	vector<Point2f> pts_src;
	pts_src.push_back(Point2f(0, 0));
	pts_src.push_back(Point2f(size.width - 1, 0));
	pts_src.push_back(Point2f(size.width - 1, size.height - 1));
	pts_src.push_back(Point2f(0, size.height - 1));

	// Destination image
	Mat im_dst = imread("raw.png");

	// Set data for mouse handler
	Mat im_temp = im_dst.clone();
	userdata data;
	data.im = im_temp;

	//show the image
	imshow("Image", im_temp);

	cout << "Click on four corners of a billboard and then press ENTER" << endl;
	//set the callback function for any mouse event
	setMouseCallback("Image", mouseHandler, &data);
	waitKey(0);

	// Calculate Homography between source and destination points
	Mat h = findHomography(pts_src, data.points);

	// Warp source image
	warpPerspective(im_src, im_temp, h, im_temp.size());

	// Extract four points from mouse data
	Point pts_dst[4];
	for (int i = 0; i < 4; i++)
	{
		pts_dst[i] = data.points[i];
	}
	// Black out polygonal area in destination image.
	fillConvexPoly(im_dst, pts_dst, 4, Scalar(0), CV_AA);

	// Add warped source image to destination image.
	im_dst = im_dst + im_temp;

	// Display image.
	imshow("Image", im_dst);
	waitKey(0);
	return 0;
}

int flipVideo() {
	cout << "start!!!" << endl;
	VideoCapture video(videoPath);
	if (!video.isOpened())
	{
		cout << "video open error!" << endl;
		return -1;
	}
	double fps = video.get(CV_CAP_PROP_FPS); //获取视频帧率
	cout << "video fps:" << fps << endl;

	double pauseTime = 1000 / fps; //两幅画面中间间隔
	int w = static_cast<int>(video.get(CV_CAP_PROP_FRAME_WIDTH));
	int h = static_cast<int>(video.get(CV_CAP_PROP_FRAME_HEIGHT));
	cv::Size S(w, h);

	VideoWriter write;
	write.open("video_pro.mp4", CV_FOURCC('D', 'I', 'V', 'X'), fps, S, true);
	Mat image;
	while (true)
	{
		//取视频的一帧
		video >> image;
		if (!image.data || waitKey(pauseTime) == 27)  //图像为空或Esc键按下退出播放
		{
			break;
		}
		flip(image, image, -1);
		write.write(image);
	}
	video.release();
	write.release();
	/*
	Mat firstimg;
	video.read(firstimg);
	flip(firstimg, firstimg, -1);
	imshow("first", firstimg);
	waitKey(0);*/
	return 0;
}
