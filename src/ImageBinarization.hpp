//
// Created by armin on 25.11.16.
//

#ifndef QRCODE_IMAGEBINARIZATION_H
#define QRCODE_IMAGEBINARIZATION_H

//#include <opencv2/ml.hpp> // Philipp: Besser als das include #include <ml.h> (verursachte Fehlermeldungen)
#include "ImageReader.hpp"

using namespace cv;
using namespace std;

class ImageBinarization {
public:
    Mat run(Mat image);

private:
    ImageReader reader;
    int threshold_value = 120;
    int threshold_type = CV_THRESH_OTSU;
    int const max_BINARY_value = 255;
};

#endif //QRCODE_IMAGEBINARIZATION_H
