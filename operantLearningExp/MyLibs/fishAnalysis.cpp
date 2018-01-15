#include "fishAnalysis.h"


using namespace std;
using namespace cv;

// Normalizes a given image into a value range between 0 and 255.  
Mat norm_0_255(const Mat& src)
{
	// Create and return normalized image:  
	Mat dst;
	switch (src.channels()) {
	case 1:
		cv::normalize(src, dst, 0, 255, NORM_MINMAX, CV_8UC1);
		break;
	case 3:
		cv::normalize(src, dst, 0, 255, NORM_MINMAX, CV_8UC3);
		break;
	default:
		src.copyTo(dst);
		break;
	}
	return dst;
}

void rot_90_CCW(FishData* myFish)
{
	Mat temp;
	transpose(myFish->openCvImage, temp);
	flip(temp, myFish->rotImage, 0);
}

void crop_image(FishData* myFish, cv::Rect cropRoi)
{
	Mat roiImage(myFish->rotImage, cropRoi);



	roiImage.copyTo(myFish->cropImage);// deep copy
	roiImage.copyTo(myFish->rawImg);
	roiImage.copyTo(myFish->HUDSimg);

	
	GaussianBlur(myFish->cropImage, myFish->cropImage, Size(3, 3), 0, 0);


	myFish->cropImage = 255 - myFish->cropImage;
	
	threshold(myFish->cropImage, myFish->cropImage, 20, 255, THRESH_TOZERO);

	myFish->cropImage = gamma(myFish->cropImage, 1.5);

	//Mat g_temp;
	//myFish->cropImage.copyTo(g_temp);
	//g_temp.convertTo(g_temp, CV_32FC1);
	//float gamma = 1.5;
	//float gamma_2 = 0.8;
	//pow(g_temp, gamma, g_temp);
	//pow(g_temp, gamma_2, g_temp);
	//g_temp = norm_0_255(g_temp);
	//myFish->cropImage = g_temp;

	Mat tempF64;
	myFish->cropImage.convertTo(tempF64, CV_64FC1);
	tempF64.copyTo(myFish->cropImgF64);

}

Mat getMean(const vector<Mat>& images)
{
	if (images.empty()) return Mat();
	// Create a 0 initialized image to use as accumulator
	Mat m(images[0].rows, images[0].cols, CV_64FC1);
	m.setTo(0.0);

	// Use a temp image to hold the conversion of each input image to CV_64FC1
	// This will be allocated just the first time, since all your images have
	// the same size.
	Mat temp;
	for (int i = 0; i < images.size(); i++)
	{
		// convert the input images to CV_64FC3 ...
		images[i].convertTo(temp, CV_64FC1);

		// ... so you can accumulate
		m += temp;
	}
	// Convert back to CV_8UC3 type, applying the division to get the actual mean
	m = m / images.size();
	//m.convertTo(m, CV_8UC1, 1. / images.size());
	return m;

}

Mat updateMean(Mat meanImage, Mat firstImage, Mat newImage, int numImages)
{
	Size matSize = meanImage.size();
	Mat subImage(matSize, CV_64FC1);
	Mat resImage(matSize, CV_64FC1);
	Mat diffImage(matSize, CV_64FC1);

	Mat tempNew;
	//meanImage.convertTo(meanImage, CV_64FC1);
	// substract the fist image
	resImage = meanImage - firstImage / numImages + newImage / numImages;

	return resImage;

}

void updateMean(vector<Mat>* images, FishData* myFish, int idxFrame)
{
	int numMeanImg = 200;
	int updateFreq = 30; // every 30 frame update once
	int numImages = images->size();
	Mat newImage;
	myFish->cropImgF64.copyTo(newImage);
	if (numImages < numMeanImg)
	{
		if (numImages == 0)
		{
			myFish->bgImg = newImage;
		}
		else {
			myFish->bgImg = (myFish->bgImg*numImages + newImage) / (numImages + 1);
		}
		images->push_back(newImage);
	}
	else {
		if (idxFrame%updateFreq == 0)
		{
			Mat newMean = updateMean(myFish->bgImg, images->front(), newImage, numImages);
			newMean.copyTo(myFish->bgImg);
			images->erase(images->begin());
			images->push_back(newImage);
		}
	}
}

void get_fish_pos(FishData* myFish)
{//Get fish position by image processing


	int res = findFish(myFish);

	if (res)
	{
		findFishHead(myFish);
		//cout << "Fish Head Pos: " << myFish->head << endl;
		//cout << "Heading Angle: " << myFish->headingAngle;
	}

}

int findFish(FishData* myFish)
{
	myFish->subImg = myFish->cropImgF64 - myFish->bgImg;
	myFish->frameSize = myFish->subImg.size();
	myFish->subImg.convertTo(myFish->subImg, CV_8UC1);

	threshold(myFish->cropImage, myFish->cropImage, 40, 255, THRESH_TOZERO);
	myFish->subImg = gamma(myFish->subImg, 0.95);

	GaussianBlur(myFish->subImg, myFish->subImg, Size(5, 5), 0, 0);
	threshold(myFish->cropImage, myFish->cropImage, 40, 255, THRESH_TOZERO);

	Mat temp;
	threshold(myFish->subImg, myFish->BW, myFish->threBin, 255, THRESH_BINARY);
	namedWindow("subImg", CV_WINDOW_AUTOSIZE);
	imshow("subImg", myFish->subImg);
	//waitKey(1);
	myFish->BW.copyTo(temp);
	vector<vector<Point>> contours;
	findContours(temp, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	int fishContourIndex = -1;

	vector<int> areas;
	for (int i = 0; i < contours.size(); i++)
	{
	//	if (arcLength(contours[i], true)<100)
		//{
		//	int area = contourArea(contours[i]);
			//if (area > myFish->fishContourArea_L && area < myFish->fishContourArea_H)
		//	{
				areas.push_back(contourArea(contours[i]));
		//	}
		//}
	}
	fishContourIndex = max_element(areas.begin(), areas.end()) - areas.begin();
	if (areas.empty())
		fishContourIndex == -1;

	if (fishContourIndex == -1)
	{
		myFish->head.x = -1;
		myFish->head.y = -1;

		cout << "can't find fish!!!!" << endl;

		return -1;
	}
	else {
		myFish->bigFishContour = contours[fishContourIndex];
	}

	return 1;
}

void findFishHead(FishData* myFish)
{
	const int numVertices = 4;
	RotatedRect R = minAreaRect(myFish->bigFishContour);
	Point2f BoxPoints[4];
	R.points(BoxPoints);
	Point M[2];
	Point pointArray1[numVertices];
	Point pointArray2[numVertices];
	int longEdgeIdx = 1;
	int shortEdgeIdx = 3;

	if (R.size.height < R.size.width)
	{
		longEdgeIdx = 3;
		shortEdgeIdx = 1;
	}

	M[0] = (BoxPoints[longEdgeIdx] + BoxPoints[0]) / 2;
	M[1] = (BoxPoints[shortEdgeIdx] + BoxPoints[2]) / 2;

	pointArray1[0] = BoxPoints[0];
	pointArray1[1] = M[0];
	pointArray1[2] = M[1];
	pointArray1[3] = BoxPoints[shortEdgeIdx];

	pointArray2[0] = BoxPoints[longEdgeIdx];
	pointArray2[1] = M[0];
	pointArray2[2] = M[1];
	pointArray2[3] = BoxPoints[2];

	Mat mask1(myFish->frameSize, CV_8UC1, Scalar::all(0));
	Mat mask2(myFish->frameSize, CV_8UC1, Scalar::all(0));
	fillConvexPoly(mask1, pointArray1, numVertices, Scalar(255), CV_AA, 0);
	fillConvexPoly(mask2, pointArray2, numVertices, Scalar(255), CV_AA, 0);

	Mat copy1(myFish->frameSize, CV_8UC1, Scalar::all(0));
	Mat copy2(myFish->frameSize, CV_8UC1, Scalar::all(0));
	myFish->BW.copyTo(copy1, mask1);
	myFish->BW.copyTo(copy2, mask2);

	int area1 = countNonZero(copy1);
	int area2 = countNonZero(copy2);

	Point headRefPt, tailRefPt;
	if (area1 > area2)
	{
		headRefPt = (pointArray1[0] + pointArray1[3]) / 2;
		tailRefPt = (pointArray1[1] + pointArray1[2]) / 2;
		myFish->center = (pointArray1[1] + pointArray1[2]) / 2;
	}
	else {
		headRefPt = (pointArray2[0] + pointArray2[3]) / 2;
		tailRefPt = (pointArray2[1] + pointArray2[2]) / 2;
		myFish->center = (pointArray2[1] + pointArray2[2]) / 2;
	}

	int headIndex, tailIndex;
	headIndex = find_closest_point_on_contour(myFish->bigFishContour, headRefPt);
	myFish->head = myFish->bigFishContour[headIndex];
	tailIndex = find_closest_point_on_contour(myFish->bigFishContour, tailRefPt);
	myFish->tail = myFish->bigFishContour[tailIndex];
	myFish->headingAngle = (int)calc_heading_angle(myFish->head, myFish->center);

}


int find_closest_point_on_contour(std::vector<Point>&contour, Point point)
{
	double minDist = 10000;
	double tempDist = 0;
	Point temp;
	int goodIndex;
	for (int i = 0; i < contour.size(); i++)
	{
		temp = contour[i];
		tempDist = getDistance(contour[i], point);

		if (tempDist < minDist)
		{
			goodIndex = i;
			minDist = tempDist;
		}
	}

	return goodIndex;
}

double getDistance(Point A, Point B)
{
	double distance;
	distance = sqrtf(powf((A.x - B.x), 2) + powf((A.y - B.y), 2));
	return distance;
}

void plot_contour(vector<Point>& contour, Mat& img)
{
	const Scalar color = Scalar(128);// it's a grayscale image
	const int thickness = 3;
	for (vector<Point>::iterator it = contour.begin(); it != contour.end(); it++)
	{
		if (it == contour.end() - 1)
		{
			line(img, *it, *contour.begin(), color, thickness);
		}
		else {
			line(img, *it, *(it + 1), color, thickness);
		}
	}
}

float calc_heading_angle(Point head, Point center)
{
	// Judge the quadrant
	Point vec = head - center;
	float x = vec.x;
	float y = vec.y;
	float angle = atan2(y, x) * 180 / PI; //the origin is at the top-left
	return angle;
}


//gamma correction
//input is source image and gamma param
//gamma<1, enhance dark part
//gamma>1, enhance light part
cv::Mat gamma(Mat & src, float gamma_param)
{
	Mat dst;
	src.copyTo(dst);

	dst.convertTo(dst, CV_32FC1);
	//gamma  correction
	float gamma_1 = gamma_param;
	pow(dst, gamma_1, dst);

	dst = norm_0_255(dst);

	return dst;
}

