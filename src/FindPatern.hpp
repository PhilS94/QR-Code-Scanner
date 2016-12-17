#ifndef QRCODE_FINDPATERN_H
#define QRCODE_FINDPATERN_H
#include <cmath>
#include <iostream>
#include "FinderPaternModel.hpp"


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
    Mat tiltCorrection(Mat image, FinderPaternModel fPatern);
    FinderPaternModel getFinderPaternModel(vector<Point> cont1, vector<Point> cont2, vector<Point> cont3);
    vector<FinderPaternModel> getAllPaterns();




private:
    Mat originalImage;
    //Punkte nach orienttation
    const double PI = 4.0*atan(1.0);
    double angle;
    vector<vector<Point> > contours;
    vector<FinderPaternModel> paterns;
    vector<vector<vector<Point>> >  trueContoures;
    std::vector<cv::Point> massCentres;

};
#endif //QRCODE_FINDPATERN_H
