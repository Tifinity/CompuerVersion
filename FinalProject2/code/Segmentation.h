#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/ml/ml.hpp>
#include "CImg.h"

using namespace std;
using namespace cv;
using namespace ml;
using namespace cimg_library;

struct Conn {
	vector<Point> points;
	vector<int> xy;
};

class Segmentation {
private:
	bool* visit;
	vector<Conn> connect;
	vector<vector<Conn>> conn_sort;
	vector<Conn> connect2;

	CImg<unsigned char> origin_img;					//
	CImg<unsigned char> gray_img;					//
	CImg<unsigned char> delate_img;					//
	CImg<unsigned char> result_img;					//
	CImg<unsigned char> result2_img;

	Mat origin_img_cv;
	Mat res_cv;

	int getThreshold(CImg<unsigned char> src);
	CImg<unsigned char> background(CImg<unsigned char> input);
	CImg<unsigned char> toGrayScale(CImg<unsigned char> input);
	CImg<unsigned char> delate(CImg<unsigned char> input);					//
	void dfs(int x, int y, vector<Point>& tmp);
	void find_connection();
	void process_connection();

	CImg<unsigned char> draw_info(CImg<unsigned char> input);
	CImg<unsigned char> draw_single(CImg<unsigned char> input);

public:
	Segmentation(string);
	~Segmentation();
};