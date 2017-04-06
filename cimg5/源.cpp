#include "CImg.h"
#include "canny_class.h"
#include <iostream>
#include <math.h>
#include <vector>
#include "hough.h"

using namespace std;
using namespace cimg_library;

int A_height = 424, A_width = 300;

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
		A(0, i) = p[i][0];
		A(1, i) = p[i][1];
		A(2, i) = 1;
		A(3, i) = A(4, i) = A(5, i) = 0;
		A(6, i) = -uv[i][0] * p[i][0];
		A(7, i) = -uv[i][1] * p[i][0];
		B(0, i) = p[i][0];
	}
	for (int i = 0; i < 4; i++)
	{
		A(3, i+4) = p[i][0];
		A(4, i+4) = p[i][1];
		A(5, i+4) = 1;
		A(0, i+4) = A(1, i) = A(2, i) = 0;
		A(6, i+4) = -uv[i][0] * p[i][1];
		A(7, i+4) = -uv[i][1] * p[i][1];
		B(0, i+4) = p[i][1];
	}
	
	return A.solve(B);
}

int main()
{
	//设置参数
	string filename[] = { "dataset (1).bmp", "dataset (2).bmp", "dataset (3).bmp", "dataset (4).bmp", "dataset (5).bmp", "dataset (6).bmp" };
	double thre_val[] = { 0.5,0.6,0.6,0.5,0.5,0.5 };
	float sigma[] = { 6.0f, 6.0f, 9.0f, 6.0f, 6.0f, 6.0f };
	float threshold[] = { 3.5f, 3.5f, 5.0f, 3.5f, 3.5f, 3.5f };
	
	//先进行canny处理，再进行hough变换，并存在result文件夹中
	for (int ni = 0; ni < 6; ni++)
	{
		cout << endl << filename[ni] << " : " << endl;
		canny_img img("Dataset\\" + filename[ni], sigma[ni], threshold[ni]);
		CImg<float> c_img=img.CannyDiscrete();
		CImg<float> src(("Dataset\\" + filename[ni]).c_str());
		vector<vector<double>> vertex = hough(c_img, src, thre_val[ni]);
		CImg<double> para = pro_solve(vertex);
		CImg<double> dst(A_width, A_height, 1, 3);
		int u, v;
		cimg_forXY(dst, x, y)
		{
			u = int((para(0, 0)*x + para(0, 1)*y + para(0, 2)) / (para(0, 6)*x + para(0, 7)*y + 1));
			v = int((para(0, 3)*x + para(0, 4)*y + para(0, 5)) / (para(0, 6)*x + para(0, 7)*y + 1));
			dst(x, y, 0) = src(u, v, 0);
			dst(x, y, 1) = src(u, v, 1);
			dst(x, y, 2) = src(u, v, 2);
		}
		dst.display();
	}
}