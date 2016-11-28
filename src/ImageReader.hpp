#ifndef IMAGEREADER_HPP
#define IMAGEREADER_HPP

#include <iostream>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

class ImageReader
{
public:
    static cv::Mat readImage(const std::string& filePath);

};

#endif // IMAGEREADER_HPP
