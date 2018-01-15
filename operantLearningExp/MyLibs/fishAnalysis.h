#pragma once
#ifndef _GUARD_FISHANALYSIS_H
#define _GUARD_FISHANALYSIS_H

// Include files to use OpenCV API.
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include <iostream>
#include <vector>
#include <math.h>

#define PI 3.14159265

typedef struct FishDataStruct
{
	int threBin = 40;
	int fishContourArea_L = 150;
	int fishContourArea_H = 500;
	float cutRatio = 0.15;

	cv::Mat openCvImage, rotImage, cropImage, HUDSimg, cropImgF64;
	cv::Mat bgImg; // the averaged background image
	cv::Mat subImg;
	cv::Mat BW;
	cv::Mat rawImg;

	cv::Size frameSize;
	int patternIdx = -1;// show which pattern are using
	int lastStiIdx = -1;// show the last sti pattern index

	std::vector<cv::Point> bigFishContour;
	std::vector<cv::Point> smallFishContour;

	cv::Point head = cv::Point(-1, -1);
	cv::Point tail;
	cv::Point center;

	int headingAngle = -360;

}FishData;

// Normalizes a given image into a value range between 0 and 255.  
cv::Mat norm_0_255(const cv::Mat& src);
//inverse color
cv::Mat inverse(const cv::Mat& src);

void rot_90_CCW(FishData* myFish);
void crop_image(FishData* myFish, cv::Rect cropRoi);

cv::Mat getMean(const std::vector<cv::Mat>& images);
cv::Mat updateMean(cv::Mat meanImage, cv::Mat firstImage, cv::Mat newImage, int numImages);
void updateMean(std::vector<cv::Mat>* images, FishData* myFish, int idxFrame);
void get_fish_pos(FishData* myFish);


int findFish(FishData* myFish);

void findFishHead(FishData* myFish);

int find_closest_point_on_contour(std::vector<cv::Point>&contour, cv::Point point);

double getDistance(cv::Point A, cv::Point B);
void plot_contour(std::vector<cv::Point>& contour, cv::Mat& img);
float calc_heading_angle(cv::Point head, cv::Point center);

cv::Mat gamma(cv::Mat& src, float gamma_param);

#endif // !_GUARD_FISHANALYSIS_H
