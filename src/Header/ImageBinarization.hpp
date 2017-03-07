//
// Created by armin on 25.11.16.
//

#ifndef QRCODE_IMAGEBINARIZATION_H
#define QRCODE_IMAGEBINARIZATION_H

class ImageBinarization {
public:
	enum thresholdMethod { Global = 0, LocalMean = 1, LocalGaussian =2 };

	cv::Mat run(cv::Mat image, int thresholdMethod);

	void computeSmoothing();

	void createHistogram();

	void setThresholdValue();

	bool isLightingUneven();

	void computeGlobalThreshold();

	void computeLocalThreshold(int adaptiveMethod, int blockSize, int C);

	int getMaxThresholdMethod();

	void printThresholdMethod(int thresholdMethod);


private:
	cv::Mat image;
	cv::Mat blurredImage;
	cv::Mat binarizedImage;
	cv::Mat hist;
	int histSize = 256;
	double m = (hist.rows * hist.cols) / 2;
	int bin = 0;
	int threshold_value = -1;
	int const max_BINARY_value = 255;
};

#endif //QRCODE_IMAGEBINARIZATION_H
