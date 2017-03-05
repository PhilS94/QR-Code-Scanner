#ifndef QRCODE_PATTERNORIENTATION_H
#define QRCODE_PATTERNORIENTATION_H

#include <opencv2/opencv.hpp>

class FinderPatternModel {

public:
    FinderPatternModel(cv::Point a, cv::Point b, cv::Point c);

	cv::Point topleft;
	cv::Point topright;
	cv::Point bottomleft;
};

struct FinderPattern
{
	std::vector<cv::Point> contour;
	std::vector<std::vector<cv::Point>> segments;
	std::vector<cv::Vec4f> lines;
};

struct QRCode
{
	FinderPattern topLeft;
	FinderPattern topRight;
	FinderPattern bottomLeft;

	std::vector<cv::Vec4f> hLines;
	std::vector<cv::Vec4f> vLines;

	cv::Mat corners;
	cv::Mat transform;
	cv::Mat transformedCorners;
	cv::Mat extractedImage;
	int version;
	int modules;
	cv::Point2f gridStepSize;

	cv::Mat qrcodeImage;
};


#endif //QRCODE_PATTERNORIENTATION_H
