
#include <cv.h>
#include "ImageBinarization.hpp"

Mat ImageBinarization::run(Mat image) {

    Mat binarizedImage;
    threshold(image, binarizedImage, threshold_value, max_BINARY_value,threshold_type);
    return binarizedImage;
}
