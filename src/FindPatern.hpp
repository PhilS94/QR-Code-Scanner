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
    bool isTrapez(std::vector<cv::Point> in);
    Point calculateMassCentres(std::vector<cv::Point>);
    std::vector<Point> getCoordinates(std::vector<Point> &contour);
    int getOrientation(vector<Point> pMassCentres);
    Mat rotateImage(Mat &image, double angle);
    double triangleArea(Point p0, Point p1, Point p2);
    double calculateAngle(Point p0, Point p1);


private:
    Mat originalImage;
    //Punkte nach orienttation
    Point topLeft;
    Point topRight;
    Point bottomLeft;
    const double PI = 4.0*atan(1.0);
    double angle;
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Point> massCentres;

};
#endif //QRCODE_FINDPATERN_H
