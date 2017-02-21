#include <opencv2/opencv.hpp> // Philipp: Besser als das include #include <cv.h> (verursachte Fehlermeldungen)
#include "ImageBinarization.hpp"

using namespace std;
using namespace cv;

void ImageBinarization::computeSmoothing() {
    image += Scalar(-50, -50, -50);
    blurredImage = image.clone();
    GaussianBlur(blurredImage, blurredImage, Size(3, 3), 0, 0);
}

void ImageBinarization::createHistogram() {
    bool uniform = true;
    bool accumulate = false;

    float range[] = {0, 256};
    const float *histRange = range;
    calcHist(&blurredImage, 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);
}

void ImageBinarization::setThresholdValue() {
    for (int i = 0; i < histSize && threshold_value < 0.0; ++i) {
        bin += cvRound(hist.at<float>(i));
        if (bin > m && threshold_value < 0.0)
            threshold_value = i;
    }
}

void ImageBinarization::computeThreshold() {
    threshold(blurredImage, binarizedImage, threshold_value, max_BINARY_value, threshold_type);
}

Mat ImageBinarization::run(Mat image) {

    this->image = image;
    computeSmoothing();
    createHistogram();
    setThresholdValue();
    computeThreshold();
    return binarizedImage;
}

