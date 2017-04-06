#pragma once
#include "CImg.h"
#include "canny_class.h"
#include <iostream>
#include <math.h>
#include <vector>

using namespace std;
using namespace cimg_library;

double pi = 3.14159265358979323846;

vector<vector<double>> hough(CImg<float> img, CImg<float> src, double val)
{
	//设置阈值参数
	const int angle_num = 180;
	int range_thre = 5;
	double fun_thre = 0.8;
	int point_thre = 3;
	double ang = pi / angle_num;
	int width = img._width, height = img._height;
	int X = width / 2, Y = height / 2;

	const int d = int(round(sqrt(pow(width, 2) + pow(height, 2))));

	//初始化hough空间存储矩阵
	vector<vector<int>> s;
	s.resize(angle_num);
	for (int i = 0; i < angle_num; i++)
	{
		s[i].resize(2 * d);
	}

	//计算hough空间矩阵
	int max = 0;
	cimg_forXY(img, x, y)
	{
		if (img(x, y) != 0)
		{
			for (int i = 0; i < angle_num; i++)
			{
				int p = int(round((x - X)*cos(i*ang) + (y - Y)*sin(i*ang)));
				p += d;
				if (p < 0 || (p >= 2 * d))
					continue;
				else
				{
					s[i][p]++;
					if (s[i][p] > max)
						max = s[i][p];
				}
			}
		}
	}

	//寻找hough空间中线的交点，且其交点次数大于阈值，并是一定区域内的最大值，以免受到光栅化的影响而有相似的值
	vector<pair<int, int>> lines;
	int thre = int(max*val);
	for (int i = 0; i < angle_num; i++)
	{
		for (int j = 0; j < 2 * d; j++)
		{
			bool flag = 1;
			if (s[i][j] > thre)
			{
				for (int k = 0; k < int(lines.size()); k++)
				{
					if ((abs(lines[k].first - i) < range_thre || abs((angle_num - lines[k].first) + i) < range_thre) && abs(lines[k].second - j) < range_thre)
					{
						if (s[i][j] > s[lines[k].first][lines[k].second])
						{
							lines[k].first = i;
							lines[k].second = j;
						}
						flag = 0;
					}
				}
				if (flag)
				{
					lines.push_back(make_pair(i, j));
				}
			}
		}
	}

	//由交点得到原空间的线性方程
	vector<vector<double>> fun;
	cout << "lines : " << endl;
	for (int i = 0; i < int(lines.size()); i++)
	{
		int row = lines[i].first;
		int col = lines[i].second;
		double dy = sin(row*ang), dx = cos(row*ang);
		cout << dx << "x + " << dy << "y = " << col - d + Y*dy + X*dx << endl;
		vector<double> t;
		t.push_back(dx);
		t.push_back(dy);
		t.push_back(col - d + Y*dy + X*dx);
		fun.push_back(t);
	}

	//计算交点，且在一定区域内最准确，以免重复
	CImg<int> dst(src);
	unsigned char red[] = { 255,0,0 };
	unsigned char green[] = { 0,255,0 };
	vector<vector<double>> ver;
	cimg_forXY(dst, x, y)
	{
		int sum = 0;
		double error = 0;
		for (int i = 0; i<int(fun.size()); i++)
		{
			double re = x*fun[i][0] + y*fun[i][1];
			if (re<fun[i][2] + fun_thre&&re>fun[i][2] - fun_thre)
			{
				sum++;
				error += abs(fun[i][2] - re);
				dst(x, y, 0) = 0; dst(x, y, 1) = 255; dst(x, y, 0) = 0;
			}
		}
		if (sum >= 2)
		{
			bool flag = 1;
			for (int i = 0; i<int(ver.size()); i++)
			{
				if (abs(x - ver[i][0]) < point_thre&&abs(y - ver[i][1]) < point_thre)
				{
					if (error < ver[i][2])
					{
						ver[i][0] = x;
						ver[i][1] = y;
						ver[i][2] = error;
					}
					flag = 0;
				}
			}
			if (flag)
			{
				vector<double> t;
				t.push_back(x);
				t.push_back(y);
				t.push_back(error);
				ver.push_back(t);
			}
		}
	}

	//画点
	double smax = 0;
	int six = 0, siy = 0;
	double dist;
	for (int i = 0; i<int(ver.size()); i++)
	{
		for (int j = i + 1; j<int(ver.size()); j++)
		{
			dist = sqrt(pow(ver[i][0] - ver[j][0], 2) + pow(ver[i][1] - ver[j][1], 2));
			if (dist > smax)
			{
				smax = dist;
				six = i; siy = j;
			}
		}
	}
	vector<vector<double>> order_ver;
	order_ver.resize(int(ver.size()));
	order_ver[0] = ver[six];
	order_ver[2] = ver[siy];
	int index = 1;
	for (int i=0; i<int(ver.size()); i++)
	{
		if (i != six&&i != siy)
		{
			order_ver[index] = ver[i];
			index += 2;
		}
	}
	if (sqrt(pow(order_ver[1][0] - order_ver[0][0], 2) + pow(order_ver[1][1] - order_ver[0][1], 2)) < sqrt(pow(order_ver[3][0] - order_ver[0][0], 2) + pow(order_ver[3][1] - order_ver[0][1], 2)))
	{
		vector<double> vt = order_ver[1];
		order_ver[1] = order_ver[3];
		order_ver[3] = vt;
	}
	cout << "vertex : " << endl;
	for (int i = 0; i<int(ver.size()); i++)
	{
		cout << "( " << int(order_ver[i][0]) << " , " << int(order_ver[i][1]) << " )" << endl;
	}
	dst.draw_circle(int(order_ver[0][0]), int(order_ver[0][1]), 6, red, 1);
	dst.draw_circle(int(order_ver[1][0]), int(order_ver[1][1]), 6, green, 1);
	dst.draw_circle(int(order_ver[2][0]), int(order_ver[2][1]), 6, red, 1);
	//dst.display();
	cout << endl;
	return order_ver;
}