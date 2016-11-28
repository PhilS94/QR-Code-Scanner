//
// Created by armin on 25.11.16.
//

#ifndef QRCODE_IMAGEBINARIZATION_H
#define QRCODE_IMAGEBINARIZATION_H

#include <ml.h>
#include "ImageReader.hpp"

using namespace cv;
using namespace std;

class ImageBinarization
{
public:
    Mat run(Mat image);

private:
    ImageReader reader;
    int threshold_value = 128;
    int threshold_type = THRESH_BINARY;
    int const max_BINARY_value = 255;
};
#endif //QRCODE_IMAGEBINARIZATION_H
