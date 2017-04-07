#include "CImg.h"
#include "canny_class.h"
#include <iostream>
#include <vector>
#include "hough.h"
#include <fstream>

using namespace std;
using namespace cimg_library;

int A_height = 297, A_width = 210;

//根据原图求得的A4纸四个定点与A4纸4个定点建立方程组，并求解得到系数
CImg<double> pro_solve(vector<vector<double>> p)
{
	vector<vector<double>> uv;
	uv.resize(4);
	for (int i = 0; i < 4; i++)
	{
		uv[i].resize(2);
	}
	uv[0][0] = uv[0][1] = 0;
	uv[1][0] = 0; uv[1][1] = A_height;
	uv[2][0] = A_width; uv[2][1] = A_height;
	uv[3][0] = A_width; uv[3][1] = 0;
	CImg<double> A(8, 8), B(1, 8);
	for (int i = 0; i < 4; i++)
	{
		A(0, i) = uv[i][0];
		A(1, i) = uv[i][1];
		A(2, i) = 1;
		A(3, i) = A(4, i) = A(5, i) = 0;
		A(6, i) = -uv[i][0] * p[i][0];
		A(7, i) = -uv[i][1] * p[i][0];
		B(0, i) = p[i][0];
	}
	for (int i = 0; i < 4; i++)
	{
		A(3, i+4) = uv[i][0];
		A(4, i+4) = uv[i][1];
		A(5, i+4) = 1;
		A(0, i+4) = A(1, i + 4) = A(2, i + 4) = 0;
		A(6, i+4) = -uv[i][0] * p[i][1];
		A(7, i+4) = -uv[i][1] * p[i][1];
		B(0, i+4) = p[i][1];
	}
	return B.solve(A);
}

int main()
{
	//设置参数,并获得文件名
	vector<string> filename;
	ifstream in;
	in.open("Dataset\\filename.txt");
	string st;
	while (getline(in,st))
	{
		filename.push_back(st);
	}
	int filenum = int(filename.size());
	vector<double> thre_val;
	vector<float> sigma, threshold;
	for (int i = 0; i < filenum; i++)
	{
		thre_val.push_back(0.5);
		sigma.push_back(6.0f);
		threshold.push_back(3.5f);
	}
	thre_val[1] = thre_val[2] = thre_val[10] = 0.6; thre_val[13] = 0.7;
	sigma[2]=sigma[15] = 9.0f;
	threshold[2]= threshold[15] = 5.0f;

	//先进行canny处理，再进行hough变换，并存在result文件夹中
	for (int ni =0; ni < filenum; ni++)
	{
		cout << endl << filename[ni] << " : " << endl;
		canny_img img("Dataset\\" + filename[ni], sigma[ni], threshold[ni]);
		CImg<float> c_img=img.CannyDiscrete();
		CImg<float> src(("Dataset\\" + filename[ni]).c_str());
		vector<vector<double>> vertex = hough(c_img, src, thre_val[ni]);
		CImg<double> para = pro_solve(vertex);

		//根据求得的系数，得到变换矩阵wp
		CImg<double> wp(src._width, src._height, 1, 2);
		cimg_forXY(wp, x, y)
		{
			wp(x, y, 0) = int((para(0, 0)*x + para(1, 0)*y + para(2, 0)) / (para(6, 0)*x + para(7, 0)*y + 1));
			wp(x, y, 1) = int((para(3, 0)*x + para(4, 0)*y + para(5, 0)) / (para(6, 0)*x + para(7, 0)*y + 1));
		}
		//对变换后图像进行裁剪
		(CImg<float>(("Dataset\\" + filename[ni]).c_str()),src.warp(wp).crop(0,0,A_width,A_height).display()).save(("result\\"+ filename[ni]).c_str());
	}
	return 0;
}