#include<opencv2\opencv.hpp> 
#include <math.h>
#include<vector>
#include < algorithm>

using namespace std;
using namespace cv;

int max_centers = 20;
Point *pp = new Point[max_centers*2];

struct Rad {
	int rad ;
	int countR ;
};

int getCentre(Mat edge);
int* getR(int cnum, Mat& edge);

int main() {
	Mat img = imread("circlesTest.bmp");
	imshow("原始图", img);
	if (img.empty())
		return 0;
	Mat edge;
	//将原始图转化为灰度图
	cvtColor(img, edge, COLOR_BGR2GRAY);
	//先使用3*3内核来降噪
	blur(edge, edge, Size(5, 5));
	//运行canny算子得到边缘输入图像
	Canny(edge, edge, 70, 210, 3);
	//对提取的边缘图像进行二值处理(边缘像素为1，非边缘为0)
	Mat EDGE;
	threshold(edge, EDGE, 200, 1, CV_THRESH_BINARY);

	//获得圆心的数目
	int c_num = getCentre(EDGE);
	//得到圆的半径
	cout << c_num << endl;
	int *rad = getR(c_num, EDGE);

	cout << "尺寸：" << edge.rows << ' ' << edge.cols << endl;

	for (int i = 0; i < c_num; i++) {
		if (rad[i] != 0) {
			circle(img, pp[i], rad[i], Scalar(0, 0, 255), 2, 8);
			circle(edge, pp[i], 1, 255, 2, 8);
		}
	}
	imshow("检测圆显示", img);
	imshow("edge", edge);
	waitKey(0);
	delete[]rad;
	delete[]pp;
	return 0;
}


int getCentre( Mat edge) {

	if (edge.empty())
		return 0;
	//先对边缘输入图像进行行扫描
	int col = edge.cols;//离散的格子数1
	int r = edge.rows ;

	int *rowA = new int[r];
	for (int i = 0; i < r; i++)
	{
		rowA[i] = 0;
	}
	int *colB = new int[col];
	for (int i = 0; i < col; i++)
	{
		colB[i] = 0;
	}
	
	for (int i = 0; i < edge.rows; i++)
	{
		uchar* p = edge.ptr<uchar>(i);
		for (int j = 1; j < edge.cols-1; j++)
		{
			if (p[j] == 0) {

				int minL = (j < (edge.cols - j)) ? j : (edge.cols - j);
				for (int s = 1; s <= minL;s++) {
					
					if ((p[j - s] == 1) && (p[j + s] == 1)) {
						colB[j] += 2;
						colB[j - 1] += 1;
						colB[j + 1] += 1;
						break;
					}			
				}
			}
		}
	}	

	for (int i = 0; i < edge.cols; i++)
	{	
		for (int j = 1; j < edge.rows-1; j++)
		{
			uchar t = edge.at<uchar>(j, i);
			if (t == 0) {			
				int minL = (j <= (edge.rows - j)) ? j : (edge.rows - j);
				for (int s = 1; s < minL; s++) {
					if ((edge.at<uchar>(j-s, i) == 1) && (edge.at<uchar>(j+s, i)==1)) {
						rowA[j] += 2;
						rowA[j+1] += 1;
						rowA[j-1] += 1;	
						break;
					}
				}
			}
		}
	}	
	/*int countA = 0;
	for (int i = 0; i < r; i++)
	{
		countA += rowA[i];
	}	
	int countB = 0;
	for (int i = 0; i < col; i++)
	{
		countB += colB[i];
	}*/

	int *rowAcopy = new int[r];
	for (int i = 0; i < r; i++)
	{
		rowAcopy[i] = rowA[i];
	}
	int *colBcopy = new int[col];
	for (int i = 0; i < col; i++)
	{
		colBcopy[i] = colB[i];
	}

	int max_iter = col>r?col:r;
	int *ra = new int[max_centers]();

	for (int h = 0; h < max_centers; h++) {
		//防止死循环
		max_iter--;
		if (max_iter < 0) {
			cout <<"jumpRR,h="<<h<< endl;
			break;
		}
		int max_v = 0;
		int aaa = 0;
		for (int i = 0; i < r; i++) {
			if (rowA[i] > max_v) {
				max_v = rowA[i];
				aaa = i;
			}
		}
		if (h == 0) {
			rowA[aaa] = 0;
			ra[h] = aaa;
			cout<<h << ":" << aaa << endl;
		}
		else {
			for (int nn = 0; nn <= h - 1; nn++) {
				if (abs(ra[nn] - aaa) <= 3) {
					rowA[aaa] = 0;
					h--;
					break;
				}
			}		
			if(rowA[aaa] != 0) {
				rowA[aaa] = 0;
				ra[h] = aaa;
				cout <<h<<":" <<aaa << endl;
			}
		}
	}

	int *cb = new int[max_centers]();
	max_iter = col > r ? col : r;
	for (int h = 0; h < max_centers; h++) {
		//防止死循环
		max_iter--;
		if (max_iter < 0) {
			cout << "jumpCC,h=" <<h<< endl;
			break;
		}
		
		int max_v = 0;
		int aaa = 0;
		for (int i = 0; i < col; i++) {
			if (colB[i] > max_v) {
				max_v = colB[i];
				aaa = i;
			}
		}
		if (h == 0) {
			colB[aaa] = 0;
			cb[h] = aaa;
			cout<<h << ":" << aaa << endl;
		}
		else {
			//根据实验调整

			for (int nn = 0; nn <= h - 1; nn++) {
				if (abs(cb[nn] - aaa) <= 3) {
					colB[aaa] = 0;
					h--;
					break;
				}
			}
			if (colB[aaa] != 0) {
				colB[aaa] = 0;
				cb[h] = aaa;
				cout << h << ":" << aaa << endl;
			}
		}
	}
	//构造二维数组进行辅助判断找出最可能为圆心的点

	int **array = new int*[max_centers];//分配一个指针数组，将其首地址保存在a中   、
	
	for (int i = 0; i < max_centers; i++)//为指针数组的每个元素分配一个数组
		array[i] = new int[max_centers];
	
	for (int i = 0; i < max_centers; i++) {
		for (int j = 0; j < max_centers; j++) {
			array[i][j] = rowAcopy[ra[i]]+colBcopy[cb[j]];
		}
	}

	pp[0].x =cb[0];
	pp[0].y =ra[0];	
	array[0][0] = 0;
	//number = (num_c<num_r)?num_c:num_r;
	for (int kl = 1; kl < max_centers*2; kl++) {
		int max_v = 0;
		int x = 0, y = 0;
		for (int i = 0; i < max_centers; i++) {
			for (int j = 0; j < max_centers; j++) {

				if (array[i][j] > max_v) {
					max_v = array[i][j];
					x = i;
					y = j;
				}
			}
			
		}
		array[x][y] = 0;
		pp[kl].x = cb[y];
		pp[kl].y = ra[x];
	}

	for (int i = 0; i < max_centers*2; i++) {
		cout <<"Point:"<< pp[i].x << ',' << pp[i].y << endl;
	}

	return max_centers*2;
	//票数占总票数的0.1以上则可认为是圆心
	waitKey(0);
	delete[]ra;
	delete[]cb;
	delete[] rowA,rowAcopy;
	delete[] colB,colBcopy;

}

int* getR( int cnum,Mat& edge ) {

	int *rRr = new int[cnum];

	for (int s = 0; s < cnum; s++) {
		
		vector<Rad>  RR;//临时存储半径的向量
		Rad x = { 0,0 };
		RR.push_back(x);
		
		for (int i = 0; i < edge.rows; i++)
		{
			uchar* p = edge.ptr<uchar>(i);

			for (int j = 0; j < edge.cols; j++)
			{
				if (p[j] == 1) {
					//统计边缘像素的个数
					
					int rr = int(sqrt(pow((i - pp[s].y), 2) + pow((j - pp[s].x), 2)) + 0.5);//四舍五入
					bool test = true;
					//倒序遍历加快速度
					for (vector<Rad>::iterator it = RR.end() - 1; it >= RR.begin(); it--)
					{
						if (rr == it->rad) {
							it->countR += 1;
							test = false;
							break;
						}
						if (it == RR.begin())
							break;
					}
					if(test==true){
						Rad newR = { rr,1 };
						RR.push_back(newR);
					}
				}
			}
		}
		vector<Rad>::iterator it;//声明一个迭代器，来访问vector容器，作用：遍历或者指向vector容器的元素 
		int max_count = 0;
		for (it = RR.begin(); it != RR.end(); it++)
		{
			if (it->countR > max_count) {
				max_count = it->countR;
				rRr[s] = it->rad;
			}
		}
		if (rRr[s] >= (edge.cols/2) || rRr[s] >= (edge.rows/2)) {
			rRr[s] = 0;
			cout << "半径过大无效圆" << endl;
		}
			
		int isCircle = 0.8 * 3 * rRr[s];//根据实验调整
		if (max_count <= isCircle) {
			rRr[s] = 0;
			cout << "一个无效圆+1" << endl;
		}		
		cout << "第" << s << "个半径已经计算完成！"<<rRr[s] << endl;
	}
	return rRr;
}

