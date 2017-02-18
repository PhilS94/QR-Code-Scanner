//
// Created by armin on 25.11.16.
//

#ifndef QRCODE_IMAGEBINARIZATION_H
#define QRCODE_IMAGEBINARIZATION_H

using namespace cv;
using namespace std;

class ImageBinarization {
public:
    Mat run(Mat image);

    void computeSmoothing();

    void createHistogram();

    void setThresholdValue();

    void computeThreshold();

private:
    Mat image;
    Mat blurredImage;
    Mat binarizedImage;
    Mat hist;
    int histSize = 256;
    double m = (hist.rows * hist.cols) / 2;
    int bin = 0;
    int threshold_value = -1;
    int threshold_type = CV_THRESH_OTSU;
    int const max_BINARY_value = 255;
};

#endif //QRCODE_IMAGEBINARIZATION_H
