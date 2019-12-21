#include "Segmentation.h"
#include <sstream>

using namespace std;

Segmentation::Segmentation(string filename) {
	origin_img_cv = imread(filename);
	CImg<unsigned char> tmp(filename.c_str());
	origin_img = tmp;
	if (!origin_img.data()) {
		cout << "Could not open or find the image" << std::endl;
		exit(-1);
	}

	visit = new bool[origin_img._width * origin_img._height];
	for (int i = 0; i < origin_img._width * origin_img._height; i++)
		visit[i] = false;

	origin_img.display("origin image");
	delate_img = toGrayScale(origin_img);
	delate_img.display("gray_img image");
	find_connection();
	process_connection();
	//result_img = draw_info(origin_img);
	result_img = draw_single(origin_img);
	result_img.display("result image");
	string s = string("./result/");
	string filepath = s + filename;
	result_img.save(filepath.c_str());

	//process_connection();
	//getdata(origin_img_cv);

	//result_img = draw_red(origin_img);
	//result_img = draw_blue(result_img);
	//result_img = draw_green(result_img);
	//result_img.display("result image");

	//knn_red();
	//knn_blue();
	//knn_green();
	//result_img = find_brackets(result_img);
	//result_img.display("result image");
	//result_img.save("1.bmp");	
}

Segmentation::~Segmentation() {
	delete[]visit;
}

bool mycmpx(Conn a, Conn b) {
	return a.xy[0] < b.xy[0];
}

bool mycmpy(Conn a, Conn b) {
	return a.xy[2] < b.xy[2];
}

CImg<unsigned char> Segmentation::toGrayScale(CImg<unsigned char> input) {
	CImg<unsigned char> grayscaled = CImg<unsigned char>(input._width, input._height);
	cimg_forXY(input, x, y) {
		int b = input(x, y, 0);
		int g = input(x, y, 1);
		int r = input(x, y, 2);
		double newValue = (r * 0.2126 + g * 0.7152 + b * 0.0722);
		grayscaled(x, y) = newValue;
	}
	cimg_forXY(grayscaled, x, y) {
		if (grayscaled(x, y) < 180) {
			grayscaled(x, y) = 0;
		}
		else {
			grayscaled(x, y) = 255;
		}
	}
	return grayscaled;
}

void Segmentation::dfs(int x, int y, vector<Point>& tmp) {
	if (x < 0 || x >= delate_img._width ||
		y < 0 || y >= delate_img._height ||
		visit[y * delate_img._width + x] ||
		delate_img(x, y) != 0)
		return;

	Point tmpp = Point(x, y);
	tmp.push_back(tmpp);
	visit[y * delate_img._width + x] = true;

	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			if (i != 0 || j != 0) {
				dfs(x + i, y + j, tmp);
			}
		}
	}
}

void Segmentation::find_connection() {
	cimg_forXY(delate_img, x, y) {
		if (delate_img(x, y) == 0 && visit[y * delate_img._width + x] == false) {
			vector<Point> tmp;
			dfs(x, y, tmp);
			int xmin = origin_img._width, xmax = 0;
			int ymin = origin_img._height, ymax = 0;
			for (int j = 0; j < tmp.size(); j++) {
				xmin = tmp[j].x < xmin ? tmp[j].x : xmin;
				xmax = tmp[j].x > xmax ? tmp[j].x : xmax;
				ymin = tmp[j].y < ymin ? tmp[j].y : ymin;
				ymax = tmp[j].y > ymax ? tmp[j].y : ymax;
			}
			vector<int> xytmp;
			xytmp.push_back(xmin);
			xytmp.push_back(xmax);
			xytmp.push_back(ymin);
			xytmp.push_back(ymax);

			Conn tmptmp;
			tmptmp.points = tmp;
			tmptmp.xy = xytmp;

			connect.push_back(tmptmp);
		}
	}
}

void Segmentation::process_connection() {
	sort(connect.begin(), connect.end(), mycmpy);
	int tmpy = 136;
	vector<Conn> tmp_con_y;
	for (int i = 0; i < connect.size(); i++) {
		if (connect[i].points.size() < 200 || connect[i].points.size() > 10000) continue;
		if (abs(connect[i].xy[2] - tmpy) > 55) {
			conn_sort.push_back(tmp_con_y);
			tmpy = connect[i].xy[2];
			tmp_con_y.clear();
		}
		tmp_con_y.push_back(connect[i]);
	}
	conn_sort.push_back(tmp_con_y);
	
	for (int i = 0; i < conn_sort.size(); i++) {
		sort(conn_sort[i].begin(), conn_sort[i].end(), mycmpx);
		for (int j = 0; j < conn_sort[i].size(); ++j) {
			Conn tmp = conn_sort[i][j];
			if (j == conn_sort[i].size() - 1) {
				connect2.push_back(tmp);
				break;
			}
			for (int k = j + 1; k < conn_sort[i].size(); ++k) {
				if (conn_sort[i][k].xy[0] - tmp.xy[1] < 4) {
					if (conn_sort[i][k].xy[0] < tmp.xy[0]) tmp.xy[0] = conn_sort[i][k].xy[0];
					if (conn_sort[i][k].xy[1] > tmp.xy[1]) tmp.xy[1] = conn_sort[i][k].xy[1];
					if (conn_sort[i][k].xy[2] < tmp.xy[2]) tmp.xy[2] = conn_sort[i][k].xy[2];
					if (conn_sort[i][k].xy[3] > tmp.xy[3]) tmp.xy[3] = conn_sort[i][k].xy[3];
					if (k == conn_sort[i].size() - 1) {
						connect2.push_back(tmp);
					}
				}
				else {
					connect2.push_back(tmp);
					j = k - 1;
					break;
				}
			}
		}
	}
}

CImg<unsigned char> Segmentation::draw_single(CImg<unsigned char> input) {
	CImg<unsigned char> result = input;
	const unsigned char red[] = { 255,0,0 };
	for (int i = 0; i < connect2.size(); i++) {
		int x0 = connect2[i].xy[0];
		int x1 = connect2[i].xy[1];
		int y0 = connect2[i].xy[2];
		int	y1 = connect2[i].xy[3];
		result.draw_line(x0, y0, x1, y0, red);
		result.draw_line(x0, y1, x1, y1, red);
		result.draw_line(x0, y0, x0, y1, red);
		result.draw_line(x1, y0, x1, y1, red);
	}
	return result;
}

CImg<unsigned char> Segmentation::draw_info(CImg<unsigned char> input) {
	CImg<unsigned char> result = input;
	const unsigned char red[] = { 255,0,0 };
	const unsigned char green[] = { 0,255,0 };
	const unsigned char blue[] = { 0,0,255 };
	const unsigned char yellow[] = { 255,255,0 };
	const unsigned char r1[] = { 255,0,255 };
	const unsigned char r2[] = { 0,255,255 };

	sort(connect.begin(), connect.end(), mycmpy);
	int x0 = origin_img._width, x1 = 0;
	int y0 = origin_img._height, y1 = 0;
	int tmpy = 136;
	int count = 0;
	for (int i = 0; i < connect.size(); i++) {
		int xmin = origin_img._width, xmax = 0;
		int ymin = origin_img._height, ymax = 0;
		if (connect[i].points.size() < 50 || connect[i].points.size() > 15000) continue;
		for (int j = 0; j < connect[i].points.size(); j++) {
			xmin = connect[i].points[j].x < xmin ? connect[i].points[j].x : xmin;
			xmax = connect[i].points[j].x > xmax ? connect[i].points[j].x : xmax;
			ymin = connect[i].points[j].y < ymin ? connect[i].points[j].y : ymin;
			ymax = connect[i].points[j].y > ymax ? connect[i].points[j].y : ymax;
		}	
		if (abs(ymin - tmpy) > 65) {
			tmpy = ymin;
			if (count == 0) {
				result.draw_line(x0, y0, x1, y0, red);
				result.draw_line(x0, y1, x1, y1, red);
				result.draw_line(x0, y0, x0, y1, red);
				result.draw_line(x1, y0, x1, y1, red);
			}
			else if(count == 1) {
				result.draw_line(x0, y0, x1, y0, blue);
				result.draw_line(x0, y1, x1, y1, blue);
				result.draw_line(x0, y0, x0, y1, blue);
				result.draw_line(x1, y0, x1, y1, blue);
			}
			else if (count == 2) {
				result.draw_line(x0, y0, x1, y0, yellow);
				result.draw_line(x0, y1, x1, y1, yellow);
				result.draw_line(x0, y0, x0, y1, yellow);
				result.draw_line(x1, y0, x1, y1, yellow);
			}
			else if (count == 4) {
				result.draw_line(x0, y0, x1, y0, green);
				result.draw_line(x0, y1, x1, y1, green);
				result.draw_line(x0, y0, x0, y1, green);
				result.draw_line(x1, y0, x1, y1, green);
			}
			else if (count == 5) {
				result.draw_line(x0, y0, x1, y0, r1);
				result.draw_line(x0, y1, x1, y1, r1);
				result.draw_line(x0, y0, x0, y1, r1);
				result.draw_line(x1, y0, x1, y1, r1);
			}
			else if (count == 6) {
				result.draw_line(x0, y0, x1, y0, r2);
				result.draw_line(x0, y1, x1, y1, r2);
				result.draw_line(x0, y0, x0, y1, r2);
				result.draw_line(x1, y0, x1, y1, r2);
			}
			else {
				result.draw_line(x0, y0, x1, y0, red);
				result.draw_line(x0, y1, x1, y1, red);
				result.draw_line(x0, y0, x0, y1, red);
				result.draw_line(x1, y0, x1, y1, red);
			}
			x0 = origin_img._width;
			x1 = 0;
			y0 = origin_img._height;
			y1 = 0;
			count++;
		}
		x0 = xmin < x0 ? xmin : x0;
		x1 = xmax > x1 ? xmax : x1;
		y0 = ymin < y0 ? ymin : y0;
		y1 = ymax > y1 ? ymax : y1;
	}
	return result;
}