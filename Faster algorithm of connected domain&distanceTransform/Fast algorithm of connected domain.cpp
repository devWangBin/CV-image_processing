#include <opencv2/core/core.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<iostream>
#include <vector>

using namespace std;
using namespace cv;

int scanImg(const Mat& inputImage, vector<int>& labelImg);
void bepaint(const Mat& simg, Mat& dimg, vector<int>& labelImg);
void getMaxRegion(vector<int>& LabelImg, const Mat& src, Mat& dst, int labelcount);

int main() {
	//先对输入原图二值化
	Mat img = imread("00.png", 0);
	Mat binaryImg = img.clone();
	threshold(img, binaryImg, 127, 1, 0);

	namedWindow("原二值图");
	imshow("原二值图", img);

	vector<int> labelImg;

	int labelNumber = scanImg(binaryImg, labelImg);//一次扫描得到标签填充的vector图像
	Mat Pimg;
	bepaint(binaryImg, Pimg, labelImg);
	namedWindow("图");
	imshow("图", Pimg);
	cout << labelNumber << endl;
	Mat maxc;
	getMaxRegion(labelImg, binaryImg, maxc, labelNumber);

	namedWindow("最大连通域图");
	imshow("最大连通域图", maxc);
	waitKey(0);
	return 0;
}


struct label {
	int lanum;
	int curCount;
};


int scanImg(const Mat& inputImage, vector<int>& labelImg)
{
	//图像为空或者不是二值图像时直接返回
	if (inputImage.empty() || inputImage.type() != CV_8UC1)
	{
		return -1;
	}
	int labelNum = 1;
	int count = 1;
	int cols = inputImage.cols;
	cout << inputImage.rows << ' ' << inputImage.cols << endl;

	//图像第一行单独处理
	const uchar* rData = inputImage.ptr<uchar>(0);
	
	if (rData[0] == 1)//为白色目标区域像素
	{
		labelImg.push_back(labelNum);
	}
	else {
		labelImg.push_back(-1);
	}
	for (int j = 1; j < cols; j++)
	{
		if(rData[j]==0)
			labelImg.push_back(-1);
		else 
		{
			if( 1 == rData[j - 1])
				labelImg.push_back(labelNum);
			else {
				labelImg.push_back(++labelNum);
				count++;
			}
				
		}
	}
	cout << labelImg.size() << endl;
	//余下rows-1行
	for (int i = 1; i < inputImage.rows; i++)
	{
		const uchar* lastrowData = inputImage.ptr<uchar>(i-1);//上一行的行指针
		const uchar* rowData = inputImage.ptr<uchar>(i);//本行的行指针
		if(rowData[0]==0)
			labelImg.push_back(-1);
		else {
			if ( 1 == lastrowData[0])
				labelImg.push_back(labelImg[(i-1)*cols]);
			else {
				count++;
				labelImg.push_back(++labelNum);
			}			
		}	
		for (int j = 1; j < cols; j++)
		{		
			int a = labelImg[(i - 1)*cols + j];
			int b = labelImg[i*cols + j - 1];
			if (rowData[j] == 0)
				labelImg.push_back(-1);
			else {
				if (0 == rowData[j - 1] && 0 == lastrowData[j]) {
					count++;
					labelImg.push_back(++labelNum);
				}					
				else if (1 == rowData[j - 1] && 0 == lastrowData[j])
					labelImg.push_back(b);

				else if (0== rowData[j - 1] && 1 == lastrowData[j])
					labelImg.push_back(a);

				else {
					if (a != b) {
						for (size_t k = 0; k < labelImg.size(); k++)
						{
							if (labelImg[k] == b)
								labelImg[k] = a;
						}
						labelImg.push_back(a);
						count--;
					}
					else {

						labelImg.push_back(a);
					}		
				}
			}			
		}
	}
	cout << labelImg.size() << endl;
	return count;
}

void bepaint(const Mat& simg,Mat& dimg, vector<int>& labelImg) {
	int rows = simg.rows;
	int cols = simg.cols;

	dimg.release();
	dimg.create(rows, cols, CV_8UC3);
	dimg = Scalar::all(0);

	for (int i = 0; i < rows; i++) {

		for (int j = 0; j < cols; j++) {
			int x = labelImg[i*cols + j];

			if (x == -1) {
				dimg.at<Vec3b>(i, j)[0] = 255;
				dimg.at<Vec3b>(i, j)[1] = 255;
				dimg.at<Vec3b>(i, j)[2] = 255;
			}
			else{
				dimg.at<Vec3b>(i, j)[0] = (x*x*x*x) % 255;
				dimg.at<Vec3b>(i, j)[1] = (x*x*x) % 255;
				dimg.at<Vec3b>(i, j)[2] = (x*x) % 255;
			}				
		}
	}	
}


void getMaxRegion(vector<int>& LabelImg, const Mat& src,Mat& dst,int labelcount) {
	int rows = src.rows;
	int cols = src.cols;
	dst.release();
	dst.create(rows, cols, CV_8UC3);
	dst = Scalar::all(0);

	label *num = new label[labelcount];
	for (int i = 0; i < labelcount; i++) {
			num[i].lanum = 0;
			num[i].curCount = 0;
	}
		
	int count = 0;
	bool judge = false;

	for (size_t k = 0; k < LabelImg.size(); k++)
	{
		judge = false;
		if (LabelImg[k] > 0) {
			for (int i = 0; i < labelcount; i++)
			{
				if (LabelImg[k] == num[i].lanum) {
					judge = true;
					num[i].curCount++;
				}					
			}
			if (judge == false) {
				num[count].lanum = LabelImg[k];
				num[count].curCount++;
				count++;
			}
		}
	}
	label maxLabel;
	maxLabel.curCount = 0;
	maxLabel.lanum = 0;
	for (int i = 0; i < labelcount; i++) {
		//cout << num[i].lanum << ' ' << num[i].curCount << endl;
		if (num[i].curCount > maxLabel.curCount) {
			maxLabel.lanum = num[i].lanum;
			maxLabel.curCount = num[i].curCount;
		}
			
	}

	for (int i = 0; i < rows; i++) {

		for (int j = 0; j < cols; j++) {
			int x = LabelImg[i*cols + j];

			if (x != maxLabel.lanum) {

				dst.at<Vec3b>(i, j)[0] = 255;
				dst.at<Vec3b>(i, j)[1] = 255;
				dst.at<Vec3b>(i, j)[2] = 255;
			}
			else {

				dst.at<Vec3b>(i, j)[0] = 0;
				dst.at<Vec3b>(i, j)[1] = 0;
				dst.at<Vec3b>(i, j)[2] =255;

			}
		}
	}


	delete[]num;
}