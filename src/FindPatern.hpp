#ifndef QRCODE_FINDPATERN_H
#define QRCODE_FINDPATERN_H

using namespace cv;
using namespace std;

class FindPatern{
public:
    FindPatern(Mat originalImage);
    Mat findAllContours(Mat image);
    Mat findQRCodePaterns(Mat image);
    bool isContourInsideContour(std::vector<cv::Point> in, std::vector<cv::Point> out);
    std::vector<Point> getCoordinates(std::vector<Point> &contour);

private:
    Mat originalImage;
    std::vector<std::vector<cv::Point> > contours;
};
#endif //QRCODE_FINDPATERN_H
