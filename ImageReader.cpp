#include "ImageReader.hpp"

cv::Mat ImageReader::readImage(const std::string &filePath) {
    return cv::imread(filePath, CV_LOAD_IMAGE_ANYCOLOR);
}
