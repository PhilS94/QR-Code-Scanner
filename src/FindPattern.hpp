#ifndef QRCODE_FINDPATTERN_H
#define QRCODE_FINDPATTERN_H

#include <cmath>
#include <iostream>
#include "FinderPatternModel.hpp"

class FindPattern {
public:
    FindPattern(cv::Mat originalImage);

	cv::Mat findAllContours(cv::Mat image);

	cv::Mat findQRCodePatterns(cv::Mat image);

    bool isContourInsideContour(std::vector<cv::Point> in, std::vector<cv::Point> out);

    bool isTrapez(std::vector<cv::Point> in);

	cv::Point calculateMassCentres(std::vector<cv::Point>);

	cv::Mat tiltCorrection(cv::Mat image, FinderPatternModel fPattern);

	cv::Mat normalize(cv::Mat image);

    int getVersionNumber(cv::Mat image);

    int getModules(int versionNumber);

    FinderPatternModel getFinderPatternModel(std::vector<cv::Point> cont1, std::vector<cv::Point> cont2, std::vector<cv::Point> cont3);

    void getAllPatterns(std::vector<FinderPatternModel> &patterns);


private:
	cv::Mat originalImage;
    //Punkte nach orienttation
    const double PI = 4.0 * atan(1.0);
    double angle;
	std::vector<std::vector<cv::Point> > contours;
	std::vector<FinderPatternModel> patterns;
	std::vector<std::vector<std::vector<cv::Point>>> trueContoures;
    std::vector<cv::Point> massCentres;

};

#endif //QRCODE_FINDPATTERN_H
