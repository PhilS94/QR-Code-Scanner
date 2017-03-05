#ifndef QRCODE_PATTERNORIENTATION_H
#define QRCODE_PATTERNORIENTATION_H

#include <opencv2/opencv.hpp>

struct FinderPattern {
    std::vector<cv::Point> contour;
    std::vector<std::vector<cv::Point>> segments;
    std::vector<cv::Vec4f> lines;
};

struct QRCode {
    FinderPattern topLeft;
    FinderPattern topRight;
    FinderPattern bottomLeft;

    std::vector<cv::Vec4f> hLines;
    std::vector<cv::Vec4f> vLines;

    cv::Mat corners;
    cv::Mat transform;
    cv::Mat transformedCorners;
    cv::Mat extractedImage;
};

#endif //QRCODE_PATTERNORIENTATION_H
