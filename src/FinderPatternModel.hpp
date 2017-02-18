#ifndef QRCODE_PATTERNORIENTATION_H
#define QRCODE_PATTERNORIENTATION_H

#include "FinderPatternModel.hpp"
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

class FinderPatternModel {

public:
    FinderPatternModel(Point a, Point b, Point c);

    Point topleft;
    Point topright;
    Point bottomleft;
};


#endif //QRCODE_PATTERNORIENTATION_H
