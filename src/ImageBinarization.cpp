#include <opencv2/opencv.hpp> // Philipp: Besser als das include #include <cv.h> (verursachte Fehlermeldungen)
#include "ImageBinarization.hpp"

Mat ImageBinarization::run(Mat image) {
    Mat binarizedImage;
    Mat blurredImage;
    image += Scalar(-50, -50, -50);
    blurredImage = image.clone();
    GaussianBlur(blurredImage, blurredImage, Size(3, 3), 0, 0);
    threshold(blurredImage, binarizedImage, threshold_value, max_BINARY_value, threshold_type);
    return binarizedImage;
}
