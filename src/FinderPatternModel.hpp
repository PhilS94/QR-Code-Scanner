#ifndef QRCODE_PATTERNORIENTATION_H
#define QRCODE_PATTERNORIENTATION_H

#include "FinderPatternModel.hpp"
#include <opencv2/opencv.hpp>

class FinderPatternModel {

public:
    FinderPatternModel(cv::Point a, cv::Point b, cv::Point c);

	cv::Point topleft;
	cv::Point topright;
	cv::Point bottomleft;
};


#endif //QRCODE_PATTERNORIENTATION_H
