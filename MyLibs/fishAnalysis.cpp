/*
* Copyright 2018 Wenbin Yang <bysin7@gmail.com>
* This file is part of BLITZ (Behavioral Learning In The Zebrafish),
* which is adapted from MindControl (Andrew Leifer et al <leifer@fas.harvard.edu>
* Leifer, A.M., Fang-Yen, C., Gershow, M., Alkema, M., and Samuel A. D.T.,
* 	"Optogenetic manipulation of neural activity with high spatial resolution in
*	freely moving Caenorhabditis elegans," Nature Methods, Submitted (2010).
*
* BLITZ is a free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the license, or
* (at your option) any later version.
*
* Filename: fishAnalysis.cpp
* Abstract: this file contains all function definitions
*			used in analyzing fish behavioral parameters, such as
*			fish's head, tail and center.
*
* Current Version: 2.0
* Author: Wenbin Yang <bysin7@gmail.com>
* Modified on: Apr. 28, 2018

* Replaced Version: 1.1
* Author: Wenbin Yang <bysin7@gmail.com>
* Created on: Jan. 1, 2018
*/


// Include user-defined libraries
#include "fishAnalysis.h"

// Include standard libraries
#include <iostream>

// User-defined macros
#define PI 3.14159

using namespace std;
using namespace cv;

/* find all fish contours in the arena at the same time
 by finding the largest #fish contours in all contours.
 Involved parameters: 
	1.Threshold for contour size,
	2.Moments of contours
Scheme for fish positions in arena
|		|		|
|	0	|	1	|
|		|		|
|---------------|
|		|		|
|	2	|	3	|
|		|		|

TODO:
1. Abolish fishFlag?
2. Consize the recursive ifs
*/

void ArenaData::initialize(vector<string> fishIDs, int fishAge, vector<int> yDivs)
{
	const int historyLen = 2000; // used in MOG subtractor
	pMOG = cv::createBackgroundSubtractorMOG2(historyLen, binThre, false);
	for (int i = 0; i < numFish; i++)
	{
		FishData fish(fishIDs[i], fishAge, yDivs[i]);
		allFish.push_back(fish);
	}
}

bool ArenaData::findAllFish()
{
	bool fishFlag = false;
	// outer contours are counter clockwise
	vector<vector<Point>> contours;
	findContours(subImg, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE); 

	// record largest contour of each quadratile and its index
	// with initial values of -1 (not found)
	vector<vector<int>> maxContours;
	maxContours.resize(2, vector<int>(4, -1));

	// the size of fish contour should above this threshold
	const int conThre = 20;
	for (int i = 0; i < contours.size(); i++)
	{
		vector<Point> tempContour = contours[i];
		int contourSize = tempContour.size();
		if (contourSize < conThre)
			continue;

		Moments M = moments(tempContour); // to find the center
		int x = int(M.m10 / M.m00);
		int y = int(M.m01 / M.m00);
		Point center = Point(x, y);

		int signNum = (x - X_CUT > 0) + 2 * (y - Y_CUT > 0);
		int qIdx = 0; // Quadrantic index
		switch (signNum)
		{
		case 0:// No.0 Quadrant
			qIdx = 0;
			break;
		case 1:
			qIdx = 1;
			break;
		case 2:
			qIdx = 2;
			break;
		case 3:
			qIdx = 3;
			break;
		default:
			cout << "Fish Analysis Error" << endl;
			exit(0);
		}
		if (maxContours[0][qIdx] < contourSize)
		{
			maxContours[0][qIdx] = contourSize;
			maxContours[1][qIdx] = i;
		}
	}

	// assign largest contour to each fish
	for (int i = 0; i < numFish; i++)
	{
		if (maxContours[1][i] == -1)
		{
			allFish[i].pauseFrames++;
			cout << "Fish: " << i << " not found! " << endl;
			fishFlag = false;
		}
		else
		{
			allFish[i].pauseFrames = 0;
			allFish[i].fishContour = contours[maxContours[1][i]];
			allFish[i].findPosition();
			fishFlag = true;
		}
	}


	return fishFlag;
	
}
/*
	Find the head, center, tail and headingAngle of the fish
	by finding the end-points of the contour
*/
void FishData::findPosition()
{
	Vec4f lineVec;
	fitLine(fishContour, lineVec, CV_DIST_L2, 0, 0.01, 0.1);
	double stepSize = 100;
	vector<int> endIdx = findPtsLineIntersectContour(fishContour, 
		Point2f(lineVec[2] - stepSize * lineVec[0], lineVec[3] - stepSize * lineVec[1]), // on one side, far from the center point
		Point2f(lineVec[2] + stepSize * lineVec[0], lineVec[3] + stepSize * lineVec[1]));// on the other side, far from the center point

	Moments M = moments(fishContour); // to find the center
	
	center = Point(M.m10 / M.m00, M.m01 / M.m00);
	Point EP1 = fishContour[endIdx[0]];
	Point EP2 = fishContour[endIdx[1]];
	if (norm(EP1 - center) < norm(EP2 - center))
	{
		head = EP1; tail = EP2;
	}
	else
	{
		head = EP2; tail = EP1;
	}
	Point T2H = head - tail; // tail to head, only applied to small fish
	headingAngle = atan2(T2H.y, T2H.x) * 180 / PI;
	
}

/*
	Determine which side is fish's head 
	by measuring the area of each half
*/
bool FishData::findHeadSide(Point2f* M)
{
	vector<int> indices = findPtsLineIntersectContour(fishContour, M[0], M[2]);
	int idxMidPt = (indices[0] + indices[1]) / 2;
	Point2f midPtContour = fishContour[idxMidPt];
	double dist2line = getPt2LineDistance(M[1], M[0], M[2]);
	double dist2pt = norm(midPtContour - M[1]);

	vector<vector<Point>> contourHalves(2);
	if (indices[0] == indices[1])
	{
		cout << "The contour is too thin to cut in-between" << endl;
		return false;
	}
	else {
		// contour half of negative skewness contains refPt
		contourHalves[0].insert(contourHalves[0].end(), fishContour.begin() + indices[0], fishContour.begin() + indices[1]);
		contourHalves[1].insert(contourHalves[1].end(), fishContour.begin() + indices[1], fishContour.end());
		contourHalves[1].insert(contourHalves[1].end(), fishContour.begin(), fishContour.begin() + indices[0]);
	}

	vector<int> areas(2);
	areas[0] = contourArea(contourHalves[0]);
	areas[1] = contourArea(contourHalves[1]);

	int aIdx; // index of area where refPt is in.
	if (dist2line < dist2pt) // refPt is not in-between idx0 -> idx1
		aIdx = 1;
	else
		aIdx = 0;
	return areas[aIdx] > areas[!aIdx];
}

/*Find the closest point on the contour to the reference point, return the index findClosestPt*/
int findClosestPt(vector<Point>& contour, Point point)
{
	double minDist = 1000000;
	double tempDist = 0;
	Point temp;
	int goodIndex = 0;
	for (int i = 0; i < contour.size(); i++)
	{
		temp = contour[i];
		tempDist = norm(contour[i] - point);

		if (tempDist < minDist)
		{
			goodIndex = i;
			minDist = tempDist;
		}
	}

	return goodIndex;
}



/* Get the Euclidean distance from point P to line AB */
double getPt2LineDistance(Point2f P, Point2f A, Point2f B)
{
	Point2f BA = B - A; // the vector from A to B
	//Point2f PA = P - A; // the vector from A to P
	double dist = abs(BA.y*P.x - BA.x*P.y + B.cross(A)) / norm(BA);
	return dist;
}

/* Find 2 intersection points of a line (AB) and contour */
vector<int> findPtsLineIntersectContour(vector<Point>& contour, Point2f A, Point2f B)
{
	vector<int> goodIndices(2);
	Point2f curPt; // current point
	vector<Point> ptList; // store potential intersection points
	vector<int> idxList; // store indices of potential intersection points
	double distThre = 1.0; // threshold to decide whether it is an intersection pt
	for (int i = 0; i < contour.size(); i++)
	{
		curPt = contour[i];
		double pt2line = getPt2LineDistance(curPt, A, B);
		if ( pt2line < distThre)
		{
			ptList.push_back(contour[i]);
			idxList.push_back(i);
		}

	}

	// assign intersection points to each side
	int idxA = findClosestPt(ptList, A); // the one closest to A
	int idxB = findClosestPt(ptList, B); // the one closest to B

	if (idxA < idxB)
	{
		goodIndices[0] = idxList[idxA];
		goodIndices[1] = idxList[idxB];
	}
	else
	{
		goodIndices[0] = idxList[idxB];
		goodIndices[1] = idxList[idxA];
	}

	
	return goodIndices;

}

void rot90CW(Mat src, Mat dst)
{
	Mat temp;
	flip(src, temp, 0);
	transpose(temp, dst);
}
