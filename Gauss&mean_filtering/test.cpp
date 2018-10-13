#include<iostream>

using namespace std;

//得到图像的积分图
void  integral_image(const  uchar *src, int width, int height, int sstride, int *pint, int istride) {
	int *prow = new int[width];
	memset(prow, 0, sizeof(int)*width);
	for (int yi = 0; yi < height; ++yi, src += sstride, pint += istride) {
		prow[0] += src[0];  
		pint[0] = prow[0];//for the first pixel
		for (int xi = 1; xi < width; ++xi) 
		{ 
			prow[xi] += src[xi]; 
			pint[xi] = pint[xi - 1] + prow[xi]; 
		}
	}
	delete[]prow;
}


void GetGrayIntegralImage(unsigned char *Src, int *Integral, int Width, int Height, int Stride)
{
	memset(Integral, 0, (Width + 1) * sizeof(int));                    //    第一行都为0
	for (int Y = 0; Y < Height; Y++)
	{
		unsigned char *LinePS = Src + Y * Stride;
		int *LinePL = Integral + Y * (Width + 1) + 1;              //上一行位置
		int *LinePD = Integral + (Y + 1) * (Width + 1) + 1;            //    当前位置，注意每行的第一列的值都为0
		LinePD[-1] = 0;                                                //    第一列的值为0
		for (int X = 0, Sum = 0; X < Width; X++)
		{
			Sum += LinePS[X];                                        //    行方向累加
			LinePD[X] = LinePL[X] + Sum;                            //    更新积分图
		}
	}
}

int main() {

	int *p;
	p = new int[25];
	for (int i = 0; i <5; i++,p+=5) {
		for (int j = 0; j < 5; j++) {
			p[j] = i*5;
			cout << p[j] << endl;
		}
		
	}
	system("pause");
	return 0;
}