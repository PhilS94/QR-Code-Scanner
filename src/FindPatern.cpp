#include <ml.h>
#include <highgui.h>
#include <opencv2/features2d/features2d.hpp>
#include "FindPatern.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include <cmath>
#include <math.h>
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
//                            trueContoures.push_back((vector<Point> &&) contours.at(k));
//                            trueContoures.push_back((vector<Point> &&) contours.at(i));
                            trueContoures.push_back((vector<Point> &&) contours.at(j));
                        }
                    }
                }
            }
        }
    }

    for (int l = 0; l < trueContoures.size(); ++l) {
        massCentres.push_back(calculateMassCentres(trueContoures.at(l)));
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

    angle = getOrientation(massCentres);
    cout << "Angle: " << angle << endl;
    Mat rotatedImage;
    rotatedImage.cols = sqrt(2) * contourImage.cols * sin(angle);
    rotatedImage.rows = sqrt(2) * contourImage.rows * sin(angle);
    rotatedImage = rotateImage(contourImage, angle * 180);
   // contourImage = rotateImage(contourImage, calculateAngle(topLeft,topRight));
//    for (int l = 0; l < coordinates.size() ; ++l) {
//        cv::line(contourImage, coordinates.at(l).at(0), coordinates.at(l).at(1), color[1], 5,8,0);
//        cv::line(contourImage, coordinates.at(l).at(0), coordinates.at(l).at(2), color[1], 5,8,0);
//        cv::line(contourImage, coordinates.at(l).at(1), coordinates.at(l).at(3), color[1], 5,8,0);
//        cv::line(contourImage, coordinates.at(l).at(2), coordinates.at(l).at(3), color[1], 5,8,0);
//    }
    return rotatedImage;

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

int FindPatern::getOrientation(vector<Point> pMassCentres){



    vector<Point> temp;

    //Distanz zwischen den Punkten, um die laengste Seite zu erhalten
    double dist0_1 = abs(pMassCentres[0].x - pMassCentres[1].x) + abs(pMassCentres[0].y - pMassCentres[1].y);
    double dist1_2 = abs(pMassCentres[1].x - pMassCentres[2].x) + abs(pMassCentres[1].y - pMassCentres[2].y);
    double dist2_0 = abs(pMassCentres[2].x - pMassCentres[0].x) + abs(pMassCentres[2].y - pMassCentres[0].y);

    //slope
    double slope;

    //maximale Distanz bestimmen
    double maxDistance = max(dist0_1, max(dist1_2, dist2_0));

    cout << "dist 0-1: " << dist0_1 <<", dist 1-2: " << dist1_2<< ", dist 2-0: "<< dist2_0 << ". Max Distanz: "<< maxDistance<<endl;


    //bestimme den Punkt der oben links sein sollte
    if (maxDistance == dist0_1){
        topLeft = pMassCentres[2];
        temp.push_back(pMassCentres[0]);
        temp.push_back(pMassCentres[1]);

    } else if (maxDistance == dist1_2){
        topLeft = pMassCentres[0];
        temp.push_back(pMassCentres[1]);
        temp.push_back(pMassCentres[2]);
    } else {
        topLeft = pMassCentres[1];
        temp.push_back(pMassCentres[0]);
        temp.push_back(pMassCentres[2]);
    }



    //bestimme die Orientation der restlichen Punkten
    if (abs(topLeft.x - temp[0].x) < abs(topLeft.x - temp[1].x)) {
        bottomLeft = temp[0];
        topRight   = temp[1];
    } else {
        bottomLeft = temp[1];
        topRight   = temp[0];
    }

    cout << "TL: " << topLeft.x <<", " << topLeft.y <<endl;
    cout << "TR: " << topRight.x <<", " << topRight.y<<endl;
    cout << "BL: " << bottomLeft.x <<", " << bottomLeft.y<<endl;

    cout << "Z: " << (bottomLeft.y - topRight.y)<<endl;
    cout << "N: " << (bottomLeft.x - topRight.x)<<endl;

    double numerator = (bottomLeft.y - topRight.y);
    double denumerator = (bottomLeft.x - topRight.x);
    int sign = 1;
    if (numerator < 0 || denumerator < 0){
        sign = -1;
    }
    slope = abs(numerator) / abs(denumerator);
    slope *= sign;
    cout << "Slope: " << slope<<endl;
//
//    if (abs(topLeft.y - topRight.y) < abs(topLeft.y - bottomLeft.y)) {
//        if (topLeft.x < topRight.x) {
//            return 0;
//        } else {
//            return (-2 * 90.0 );
//        }
//    }
//
//    if (abs(topLeft.y - topRight.y) > abs(topLeft.y - bottomLeft.y)) {
//        if (topLeft.x < bottomLeft.x) {
//            return (1 * 90.0 );
//        } else {
//            return (-1 * 90.0 );
//        }
//    }


    if (slope > -1 && slope < 1) {
        return atan((slope - 1)/(1 + slope));
    } else {
        return 1 - atan(- (slope - 1)/(1 + slope));
    }

}

Mat FindPatern::rotateImage(Mat &image, double angle){

    if (image.channels() != 3 && image.channels() != 4) {
        fprintf(stderr, "Image has %d channels, cannot proceed\n", image.channels());
    }

    int rotated_width = (double)image.cols * fabs(cos(angle * CV_PI / 180.0)) + (double)image.rows * fabs(sin(angle * CV_PI / 180.0));
    int rotated_height = (double)image.rows * fabs(cos(angle * CV_PI / 180.0)) + (double)image.cols * fabs(sin(angle * CV_PI / 180.0));
    cv::Point center(image.cols / 2, image.rows / 2);
    cv::Mat rot_mat = cv::getRotationMatrix2D(center, angle, 1);
    rot_mat.at<double>(0, 2) += rotated_width / 2 - image.cols / 2;
    rot_mat.at<double>(1, 2) += rotated_height / 2 - image.rows / 2;

    cv::Mat image_rotated(rotated_height, rotated_width, (image.channels() == 3) ? CV_8UC3 : CV_8UC4, cv::Scalar::all(0));
    cv::warpAffine(image, image_rotated, rot_mat, image_rotated.size(), cv::INTER_CUBIC);

    return image_rotated;
}


double FindPatern::triangleArea(Point p0, Point p1, Point p2) {
    int matrix[3][3] = {{1, p0.x, p0.y},
                         {1, p1.x, p1.y},
                         {1, p2.x, p2.y}};

    //da es sich um eine 3x3 Matrix handelt kann die Formel von Sarrus
    //verwendet werden
    double det = (matrix[0][0] * matrix[1][1] * matrix[2][2])
                 + (matrix[0][1] * matrix[1][2] * matrix[2][0])
                 + (matrix[0][2] * matrix[1][0] * matrix[2][1])
                 - (matrix[0][2] * matrix[1][1] * matrix[2][0])
                 - (matrix[0][0] * matrix[1][2] * matrix[2][1])
                 - (matrix[0][1] * matrix[1][0] * matrix[2][2]);


    double delta = det / 2.0;

    return delta;
}

double FindPatern::calculateAngle(Point p0, Point p1){

    p0.x = p1.x;
    int sign;
    cout << "Y vergleich p0 und p1: " << p0.y << " und " << p1.y<<endl;
    cout << "X vergleich p0 und p1: " << p0.x << " und " << p1.x<<endl;
    if (p0.y < p1.y) {
        sign = -1;
    } else {
        sign = 1;
    }
    double len1 = sqrt(p0.x * p0.x + p0.y * p0.y);
    double len2 = sqrt(p1.x * p1.x + p1.y * p1.y);

    double dot = p0.x * p1.x + p0.y * p1.y;

    double a = dot / (len1 * len2);

    if (a >= 1.0)
        return 0.0;
    else if (a <= -1.0)
        return 180 * sign;
    else
        return acos(a) * 180 * sign; // 0..PI

}