#include <ml.h>
#include <highgui.h>
#include <opencv2/features2d/features2d.hpp>
#include "FindPatern.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include <regex>
#include <cv.h>

FindPatern::FindPatern(Mat originalImage) {
    this->originalImage = originalImage;
}

Mat FindPatern::findAllContours(Mat image) {

    float minPix = 8.00;
    float maxPix = 0.2 * image.cols * image.rows;

    image /= 255;
    cv::Mat contourOutput = image.clone();
    vector<Vec4i> hierarchy;
    findContours(image, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
    cout<<"contours.size = "<<contours.size()<<endl;
    int m = 0;
    while(m < contours.size()){
        if(contourArea(contours[m]) <= minPix){
            contours.erase(contours.begin() + m);
        }else if(contourArea(contours[m]) > maxPix){
            contours.erase(contours.begin() + m);
        }else ++ m;
    }
    cout<<"contours.size = "<<contours.size()<<endl;


    cv::Mat contourImage = originalImage.clone();
    cv::Scalar color[3];
    color[0] = cv::Scalar(0, 0, 255);
    color[1] = cv::Scalar(0, 255, 0);
    color[2] = cv::Scalar(255, 0, 0);

    for (size_t idx = 0; idx < contours.size(); idx++) {
        cv::drawContours(contourImage, contours, idx, color[idx % 3]);
    }
    return contourImage;
}

Mat FindPatern::findQRCodePaterns(Mat image) {

    std::vector<std::vector<cv::Point>> trueContoures;
    std::vector<std::vector<cv::Point>> coordinates;


    for (int i = 0; i < contours.size(); ++i) {
        //iteration über die Kontur Punkte
        for (int j = 0; j < contours.size(); ++j) {
            if (isContourInsideContour(contours.at(i), contours.at(j))) {
                for (int k = 0; k <contours.size(); ++k) {
                    if (isContourInsideContour(contours.at(k), contours.at(i))){
                        if(isTrapez(contours.at(j))){
                            coordinates.push_back(getCoordinates(contours.at(j)));
                            trueContoures.push_back((vector<Point> &&) contours.at(k));
                            trueContoures.push_back((vector<Point> &&) contours.at(i));
                            trueContoures.push_back((vector<Point> &&) contours.at(j));
                        }
                    }
                }
            }
        }
    }

    cv::Mat contourImage = originalImage.clone();
    cv::Scalar color[3];
    color[0] = cv::Scalar(0, 0, 255);
    color[1] = cv::Scalar(0, 255, 0);
    color[2] = cv::Scalar(255, 0, 0);
    for (size_t idx = 0; idx < trueContoures.size(); idx++) {
        cv::drawContours(contourImage, trueContoures, idx, color[idx % 3]);
        Vec3b px = contourImage.at<Vec3b>(calculateMassCentres(trueContoures.at(idx)));
        px[0] = 0;
        px[1] = 0;
        px[2] = 255;
        contourImage.at<Vec3b>(calculateMassCentres(trueContoures.at(idx))) = px;
    }

    for (int l = 0; l < coordinates.size() ; ++l) {
        cv::line(contourImage, coordinates.at(l).at(0), coordinates.at(l).at(1), color[1], 5,8,0);
        cv::line(contourImage, coordinates.at(l).at(0), coordinates.at(l).at(2), color[1], 5,8,0);
        cv::line(contourImage, coordinates.at(l).at(1), coordinates.at(l).at(3), color[1], 5,8,0);
        cv::line(contourImage, coordinates.at(l).at(2), coordinates.at(l).at(3), color[1], 5,8,0);
    }
    return contourImage;

}

std::vector<Point> FindPatern::getCoordinates(std::vector<Point> &contour){

    int minx = std::numeric_limits<int>::max();
    int miny = std::numeric_limits<int>::max();
    int maxx = std::numeric_limits<int>::min();
    int maxy = std::numeric_limits<int>::min();
    std::vector<Point> coordinates;

    for (int i = 0; i < contour.size(); ++i) {
        if(minx > contour.at(i).x){
            minx = contour.at(i).x;
        }
        if(miny > contour.at(i).y){
            miny = contour.at(i).y;
        }
        if(maxx < contour.at(i).x){
            maxx = contour.at(i).x;
        }
        if(maxy < contour.at(i).y){
            maxy = contour.at(i).y;
        }
    }

    Point topLeft(minx,maxy);
    Point topRight(maxx,maxy);
    Point bottomLeft(minx,miny);
    Point bottomRight(maxx,miny);

    coordinates.push_back(topLeft);
    coordinates.push_back(topRight);
    coordinates.push_back(bottomLeft);
    coordinates.push_back(bottomRight);

    return coordinates;
}

bool FindPatern::isContourInsideContour(std::vector<cv::Point> in, std::vector<cv::Point> out) {
    if(in.size() > 0 && out.size() > 0){
        for (int i = 0; i < in.size(); i++) {
            if (pointPolygonTest(out, in[i], false) <= 0) return false;
        }
        return true;
    }

    return false;
}

bool FindPatern::isTrapez(std::vector<cv::Point> in){

    std::vector<cv::Point> approximatedPolygon;
    double epsilon = 0.1*cv::arcLength(in,true);
    approxPolyDP(in, approximatedPolygon, epsilon, true);
    bool ret = ( approximatedPolygon.size() == 4 );
    return ret;
}

Point FindPatern::calculateMassCentres(std::vector<cv::Point> in){

    int xCoordinates[] = {std::numeric_limits<int>::max(), std::numeric_limits<int>::min()};
    int yCoordinates[] = {std::numeric_limits<int>::max(), std::numeric_limits<int>::min()};

    //extremalwerte werden bestimmt
    for(int i=0;i<in.size();i++){
        if(xCoordinates[0] > in.at(i).x){
            xCoordinates[0] = in.at(i).x;
        }
        if(xCoordinates[1] < in.at(i).x){
            xCoordinates[1] = in.at(i).x;
        }

        if(yCoordinates[0] > in.at(i).y){
            yCoordinates[0] = in.at(i).y;
        }
        if(yCoordinates[1] < in.at(i).y){
            yCoordinates[1] = in.at(i).y;
        }
    }

    int _x = (xCoordinates[0] + xCoordinates[1]) / 2;
    int _y = (yCoordinates[0] + yCoordinates[1]) / 2;

    Point point;
    point.x = _x;
    point.y = _y;

    return point;
}

