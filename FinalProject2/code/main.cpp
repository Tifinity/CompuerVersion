#include <iostream>
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/ml/ml.hpp> 

#include "Segmentation.h"

using namespace std;
using namespace cv;
using namespace ml;

void my_show(Mat src, string imgname) {
	namedWindow(imgname, 0);
	resizeWindow(imgname, 640, 480);
	imshow(imgname, src);
	waitKey(0);
}

void Correct(string input_path, string output_path) {
	Mat src_img = imread(input_path);
	my_show(src_img, "src");
	Mat gray_img, bin_img;
	// 灰度
	cvtColor(src_img, gray_img, COLOR_RGB2GRAY);
	my_show(gray_img, "gray");
	// 二值
	threshold(gray_img, bin_img, 180, 255, CV_THRESH_BINARY);
	my_show(bin_img, "bin");
	// 检测所有轮廓
	vector<vector<Point>> contours;
	findContours(bin_img, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	cout << "轮廓个数:" << contours.size() << endl;
	// 找到最大的轮廓即为名片的轮廓
	int max = 0;
	int index = -1;
	for (int i = 0; i < contours.size(); i++) {
		CvPoint2D32f rectpoint[4];
		CvBox2D rect = minAreaRect(Mat(contours[i]));
		cvBoxPoints(rect, rectpoint);
		int line1 = sqrt(
			(rectpoint[1].y - rectpoint[0].y) * (rectpoint[1].y - rectpoint[0].y) + 
			(rectpoint[1].x - rectpoint[0].x) * (rectpoint[1].x - rectpoint[0].x));
		int line2 = sqrt(
			(rectpoint[3].y - rectpoint[0].y) * (rectpoint[3].y - rectpoint[0].y) + 
			(rectpoint[3].x - rectpoint[0].x) * (rectpoint[3].x - rectpoint[0].x));
		if (line1 * line2 > max) {
			max = line1 * line2;
			index = i;
		}
	}
	// 新建只有最大轮廓的轮廓集
	vector<Point> max_contour = contours[index];
	vector<vector<Point>> max_contours;
	max_contours.push_back(max_contour);

	/*
	int xmin = src_img.cols, xmax = 0;
	int ymin = src_img.rows, ymax = 0;
	for (int i = 0; i < max_contour.size(); i++) {

	}
	Mat copy;
	src_img.copyTo(copy);

	drawContours(copy, max_contours, -1, Scalar(255,0,0), CV_FILLED);
	my_show(copy, "test");
	Point2f srcTri[4];
	Point2f dstTri[4];

	Mat warpPerspective_mat(3, 3, CV_32FC1);
	Mat warpPerspective_dst = Mat::zeros(bin_img.rows, bin_img.cols, bin_img.type());

	srcTri[0] = Point2f(0, 0);
	srcTri[1] = Point2f(bin_img.cols - 1, 0);
	srcTri[2] = Point2f(0, bin_img.rows - 1);
	srcTri[3] = Point2f(bin_img.cols - 1, bin_img.rows - 1);

	dstTri[0] = Point2f(0, 0);
	dstTri[1] = Point2f(bin_img.cols - 1, 0);
	dstTri[2] = Point2f(0, bin_img.rows - 1);
	dstTri[3] = Point2f(bin_img.cols - 1, bin_img.rows - 1);
	warpPerspective_mat = getPerspectiveTransform(srcTri, dstTri);
	warpPerspective(copy, warpPerspective_dst, warpPerspective_mat, copy.size());
	*/
	// 找到一个最小的矩形包围名片
	CvPoint2D32f rectpoint[4];
	CvBox2D rect = minAreaRect(Mat(contours[index]));
	cvBoxPoints(rect, rectpoint);
	float angle = rect.angle;
	// 如果是竖着的在角度上加九十度变成横着的
	int line1 = sqrt(
		(rectpoint[1].y - rectpoint[0].y) * (rectpoint[1].y - rectpoint[0].y) + 
		(rectpoint[1].x - rectpoint[0].x) * (rectpoint[1].x - rectpoint[0].x));
	int line2 = sqrt(
		(rectpoint[3].y - rectpoint[0].y) * (rectpoint[3].y - rectpoint[0].y) + 
		(rectpoint[3].x - rectpoint[0].x) * (rectpoint[3].x - rectpoint[0].x));
	if (line1 > line2) {
		angle = 90 + angle;
	}
	// 旋转
	Mat rotate_img(src_img.rows, src_img.cols, CV_8UC3);
	rotate_img.setTo(0);
	Point2f center = rect.center;
	Mat M2 = getRotationMatrix2D(center, angle, 1);
	warpAffine(src_img, rotate_img, M2, src_img.size(), 1, 0, Scalar(0));
	my_show(rotate_img, "rota");
	// 切割感兴趣区域
	Mat roi_img(src_img.rows, src_img.cols, CV_8UC3);
	Mat roi_mask(src_img.rows, src_img.cols, CV_8UC1);
	roi_img.setTo(0);
	roi_mask.setTo(0);
	drawContours(roi_mask, max_contours, -1, Scalar(255), CV_FILLED);
	warpAffine(roi_mask, roi_mask, M2, src_img.size(), 1, 0, Scalar(0));
	my_show(roi_mask, "mask");
	rotate_img.copyTo(roi_img, roi_mask);
	my_show(roi_img, "roi");
	// 在旋转之后的图上重新检测轮廓
	vector<vector<Point>> contours2;
	findContours(roi_mask, contours2, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	max = 0;
	index = -1;
	for (int i = 0; i < contours2.size(); i++) {
		CvPoint2D32f rectpoint[4];
		CvBox2D rect = minAreaRect(Mat(contours2[i]));
		cvBoxPoints(rect, rectpoint);
		int line1 = sqrt(
			(rectpoint[1].y - rectpoint[0].y) * (rectpoint[1].y - rectpoint[0].y) +
			(rectpoint[1].x - rectpoint[0].x) * (rectpoint[1].x - rectpoint[0].x));
		int line2 = sqrt(
			(rectpoint[3].y - rectpoint[0].y) * (rectpoint[3].y - rectpoint[0].y) +
			(rectpoint[3].x - rectpoint[0].x) * (rectpoint[3].x - rectpoint[0].x));
		if (line1 * line2 > max) {
			max = line1 * line2;
			index = i;
		}
	}
	Mat res_img = roi_img(boundingRect(Mat(contours2[index])));
	my_show(res_img, "res");
	imwrite(output_path, res_img);
	/*
	vector<vector<Point>> contours2;
	Mat raw = RatationedImg;
	Mat SecondFindImg;
	cvtColor(raw, SecondFindImg, COLOR_BGR2GRAY);
	threshold(SecondFindImg, SecondFindImg, 80, 200, CV_THRESH_BINARY);
	findContours(SecondFindImg, contours2, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	max = 0;
	index = -1;
	for (int j = 0; j < contours2.size(); j++) {
		Rect rect = boundingRect(Mat(contours2[j]));
		if (rect.area() > max) {
			max = rect.area();
			index = j;
		}
	}
	Mat dstImg = raw(boundingRect(Mat(contours2[index])));
	my_show(dstImg, "result");
	//imwrite(output_path, dstImg);*/
}

int main() {
	/*
	string input_path = "./DataSet/IMG_20191204_001325.jpg";
	string output_path = "./result/IMG_20191204_001325.jpg";
	Correct(input_path.c_str(), output_path);
	*/
	Segmentation seg("IMG_20191204_001325.bmp");
}