#include<opencv2/opencv.hpp>  
#include<opencv2/xfeatures2d.hpp>
#include <windows.h>//测试运行时间

using namespace cv;
using namespace std;
using namespace cv::xfeatures2d;

void SIFT_detect_match(Mat& src1, Mat& src2);
void SURF_detect_match(Mat& src1, Mat& src2);
void ORB_detect_match(Mat& src1, Mat& src2);

int main() {
	Mat img1 = imread("tower.jpg", 1);
	Mat img2 = imread("tower256.jpg", 1);
	
	long start01 = GetTickCount();  //开始时间
	ORB_detect_match(img1,img2);
	long finish01 = GetTickCount();   //结束时间
	long t = finish01 - start01;
	cout << "ORB运行时间为" << t <<"ms!"<< endl<<endl;

	start01 = GetTickCount();  //开始时间
	SURF_detect_match(img1, img2);
	finish01 = GetTickCount();   //结束时间
	t = finish01 - start01;
	cout << "SURF运行时间为" << t << "ms!" << endl<<endl;

	start01 = GetTickCount();  //开始时间
	SIFT_detect_match(img1, img2);
	finish01 = GetTickCount();   //结束时间
	t = finish01 - start01;
	cout << "SURF运行时间为" << t << "ms!" << endl << endl;

	system("pause");
	return 0;
}

void ORB_detect_match(Mat& src1, Mat& src2) {
	//加载复制原图片
	Mat Src1 = src1.clone();
	Mat Src2 = src2.clone();

	//提取特征并描述 
	vector<KeyPoint> keys1;
	vector<KeyPoint> keys2;
	Ptr<ORB> detector = ORB::create();

	Mat descriptorMat1, descriptorMat2;
	detector->detectAndCompute(Src1, Mat(), keys1, descriptorMat1);
	detector->detectAndCompute(Src2, Mat(), keys2, descriptorMat2);

	//保存特征检测的图片
	Mat dst1, dst2;
	drawKeypoints(Src1, keys1, dst1, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	drawKeypoints(Src2, keys2, dst2, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

	imwrite("ORB_detect_src01.jpg", dst1);
	imwrite("ORB_detect_src02.jpg", dst2);

	//特征点匹配
	cv::BFMatcher matcher;
	std::vector<DMatch> matches;
	matcher.match(descriptorMat1, descriptorMat2, matches);

	////获取好的匹配点进行筛选
	//double max_dist = 0;
	//double min_dist = 100;
	//for (int i = 0; i < descriptorMat1.rows; i++)
	//{
	//	double dist = matches[i].distance;
	//	if (dist < min_dist) min_dist = dist;
	//	if (dist > max_dist) max_dist = dist;
	//}
	//cout << "ORB-- Max dist :" << max_dist << endl;
	//cout << "ORB-- Min dist :" << min_dist << endl;

	//vector< DMatch > good_matches;
	//for (int i = 0; i < descriptorMat1.rows; i++)
	//{
	//	if (matches[i].distance < 0.2*max_dist)
	//	{
	//		good_matches.push_back(matches[i]);
	//	}
	//}

	//绘制特征匹配图 
	Mat img_matches;
	drawMatches(src1, keys1, src2, keys2, matches, img_matches,
		Scalar::all(-1), Scalar::all(-1), vector<char>(),
		DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

	imwrite("ORB_Result01.jpg", img_matches);
}

void SURF_detect_match(Mat& src1, Mat& src2) {
	//加载复制原图片
	Mat Src1 = src1.clone();
	Mat Src2 = src2.clone();

	//提取特征并描述 
	vector<KeyPoint> keys1;
	vector<KeyPoint> keys2;
	Ptr<xfeatures2d::SURF> detector = xfeatures2d::SURF::create();

	Mat descriptorMat1, descriptorMat2;
	detector->detectAndCompute(Src1, Mat(), keys1, descriptorMat1);
	detector->detectAndCompute(Src2, Mat(), keys2, descriptorMat2);

	//保存特征检测的图片
	Mat dst1, dst2;
	drawKeypoints(Src1, keys1, dst1, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	drawKeypoints(Src2, keys2, dst2, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

	imwrite("SURF_detect_src01.jpg", dst1);
	imwrite("SURF_detect_src02.jpg", dst2);

	//特征点匹配
	cv::BFMatcher matcher;
	std::vector<DMatch> matches;
	matcher.match(descriptorMat1, descriptorMat2, matches);

	//获取好的匹配点进行筛选
	double max_dist = 0;
	double min_dist = 100;
	for (int i = 0; i < descriptorMat1.rows; i++)
	{
		double dist = matches[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}
	cout << "SURF-- Max dist :" << max_dist << endl;
	cout << "SURF-- Min dist :" << min_dist << endl;

	vector< DMatch > good_matches;
	for (int i = 0; i < descriptorMat1.rows; i++)
	{
		if (matches[i].distance < 0.2*max_dist)
		{
			good_matches.push_back(matches[i]);
		}
	}

	//绘制特征匹配图 
	Mat img_matches;
	drawMatches(src1, keys1, src2, keys2, good_matches, img_matches,
		Scalar::all(-1), Scalar::all(-1), vector<char>(),
		DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

	imwrite("SURF_Result01.jpg", img_matches);
}

void SIFT_detect_match(Mat& src1, Mat& src2) {
	
		//加载复制原图片
		Mat Src1 = src1.clone();
		Mat Src2 = src2.clone();

		//提取特征并描述 
		vector<KeyPoint> keys1;
		vector<KeyPoint> keys2;
		Ptr<xfeatures2d::SIFT> detector = xfeatures2d::SIFT::create();

		Mat descriptorMat1, descriptorMat2;
		detector->detectAndCompute(Src1, Mat(), keys1, descriptorMat1);
		detector->detectAndCompute(Src2, Mat(), keys2, descriptorMat2);

		//保存特征检测的图片
		Mat dst1, dst2;
		drawKeypoints(Src1, keys1, dst1, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
		drawKeypoints(Src2, keys2, dst2, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

		imwrite("SIFT_detect_src01.jpg", dst1);
		imwrite("SIFT_detect_src02.jpg", dst2);

		//特征点匹配
		cv::BFMatcher matcher;
		std::vector<DMatch> matches;
		matcher.match(descriptorMat1, descriptorMat2, matches);

		//获取好的匹配点进行筛选
		double max_dist = 0;
		double min_dist = 100;
		for (int i = 0; i < descriptorMat1.rows; i++)
		{
			double dist = matches[i].distance;
			if (dist < min_dist) min_dist = dist;
			if (dist > max_dist) max_dist = dist;
		}
		cout << "SIFT-- Max dist :" << max_dist << endl;
		cout << "SIFT-- Min dist :" << min_dist << endl;

		vector< DMatch > good_matches;
		for (int i = 0; i < descriptorMat1.rows; i++)
		{
			if (matches[i].distance < 0.2*max_dist)
			{
				good_matches.push_back(matches[i]);
			}
		}

		//绘制特征匹配图 
		Mat img_matches;
		drawMatches(src1, keys1, src2, keys2, good_matches, img_matches,
			Scalar::all(-1), Scalar::all(-1), vector<char>(),
			DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

		imwrite("SIFT_Result01.jpg", img_matches);
}