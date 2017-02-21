#ifndef QRCODE_FINDPATTERN_H
#define QRCODE_FINDPATTERN_H

#include <cmath>
#include <iostream>
#include "FinderPatternModel.hpp"


using namespace cv;
using namespace std;

class FindPattern {
public:
    FindPattern(Mat originalImage);

    Mat findAllContours(Mat image);

    Mat findQRCodePatterns(Mat image);

    bool isContourInsideContour(std::vector<cv::Point> in, std::vector<cv::Point> out);

    bool isTrapez(std::vector<cv::Point> in);

    Point calculateMassCentres(std::vector<cv::Point>);

    Mat tiltCorrection(Mat image, FinderPatternModel fPattern);

    Mat normalize(Mat image);

    int getVersionNumber(Mat image);

    int getModules(int versionNumber);

    FinderPatternModel getFinderPatternModel(vector<Point> cont1, vector<Point> cont2, vector<Point> cont3);

    void getAllPatterns(vector<FinderPatternModel> &patterns);


private:
    Mat originalImage;
    //Punkte nach orienttation
    const double PI = 4.0 * atan(1.0);
    double angle;
    vector<vector<Point> > contours;
    vector<FinderPatternModel> patterns;
    vector<vector<vector<Point>>> trueContoures;
    std::vector<cv::Point> massCentres;

};

#endif //QRCODE_FINDPATTERN_H
