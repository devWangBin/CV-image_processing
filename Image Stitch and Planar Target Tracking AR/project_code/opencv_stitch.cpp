#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/stitching.hpp"

#include <iostream>

using namespace std;
using namespace cv;

Stitcher::Mode mode = Stitcher::PANORAMA;
vector<Mat> imgs;
string result_name = "result.jpg";

int main(int argc, char* argv[]) {

	Mat img1 = imread("01.jpg");	
	Mat img2 = imread("02.jpg");
	Mat img3 = imread("03.jpg");

	imgs.push_back(img1);	
	imgs.push_back(img2); 
	imgs.push_back(img3); 

	Mat pano;    
	Ptr<Stitcher> stitcher = Stitcher::create(mode);
	Stitcher::Status status = stitcher->stitch(imgs, pano);    
	if (status != Stitcher::OK) { 
		cout << "Can't stitch images, error code = " << status << endl;        
		return -1; 
	}     
	imwrite(result_name, pano);    
	return 0; 
}
