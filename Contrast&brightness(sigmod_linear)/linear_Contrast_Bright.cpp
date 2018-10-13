#include<opencv2/core/core.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/core/saturate.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<iostream> 

using namespace std;
using namespace cv;

int main(int, char** argv)
{
	double alpha = 1.0; /*< Simple contrast control */
	int beta = 0;       /*< Simple brightness control */

	Mat image = imread("a.png");
	//cvtColor(image, image, CV_BGR2GRAY);//תΪ�Ҷ�ͼ
	Mat new_image = Mat::zeros(image.size(), image.type());

	cout << " Basic Linear Transforms " << endl;
	cout << "-------------------------" << endl;
	cout << "* Enter the alpha value [1.0-3.0]: "; cin >> alpha;
	cout << "* Enter the beta value [0-100]: ";    cin >> beta;

	for (int y = 0; y < image.rows; y++) {
		for (int x = 0; x < image.cols; x++) {
			for (int c = 0; c < 3; c++) {
				new_image.at<Vec3b>(y, x)[c] = saturate_cast<uchar>(alpha*(image.at<Vec3b>(y, x)[c]) + beta);
			}
		}
	}

	namedWindow("Original Image", WINDOW_AUTOSIZE);
	namedWindow("New Image", WINDOW_AUTOSIZE);

	imshow("Original Image", image);
	imshow("New Image", new_image);

	waitKey();
	return 0;
}