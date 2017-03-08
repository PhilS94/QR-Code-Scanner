#ifndef QRCODE_PATTERNORIENTATION_H
#define QRCODE_PATTERNORIENTATION_H

#include <opencv2/opencv.hpp>

struct FinderPattern {
	/* The outter contour of the finder pattern. */
    std::vector<cv::Point> contour;
	/* The segments describing the four edges of the pattern*/
    std::vector<std::vector<cv::Point>> segments;
	/* The lines describing the outter edges of the pattern. */
    std::vector<cv::Vec4f> lines;
};

struct QRCode {
	/* Top left pattern. */
    FinderPattern topLeft;
	/* Top right pattern. */
    FinderPattern topRight;
	/* Bottom left pattern. */
    FinderPattern bottomLeft;

	/* Finder pattern edge lines horizontal within the correctly orientated qrcode. */
    std::vector<cv::Vec4f> hLines;
	/* Finder pattern edge lines vertical within the correctly orientated qrcode. */
    std::vector<cv::Vec4f> vLines;

	/* All hLines and vLines intersections within the original image. */
	cv::Mat corners;
	/* Perspective transform matrix. */
	cv::Mat transform;
	/* All corner intersections after transforming them. */
	cv::Mat transformedCorners;
	/* Result of perspective transform extraction applied to originalImage. */
	cv::Mat extractedImage;
	/* Detected version of the qrcode. */
	int version;
	/* Detected number of modules. */
	int modules;
	/* Step size of a single module within the extracted image.*/
	cv::Point2f gridStepSize;
	/* Percentage deduced by the verifyQRCode function. */
	float verifyPercentage;

	/* Image containing the qrcode with one module per pixel. */
	cv::Mat qrcodeImage;
};

#endif //QRCODE_PATTERNORIENTATION_H
