#include <opencv2/opencv.hpp> // Philipp: Besser als das include #include <cv.h> (verursachte Fehlermeldungen)
#include "../Header/ImageBinarization.hpp"

using namespace std;
using namespace cv;

void ImageBinarization::computeSmoothing() {
	blurredImage = image.clone();
	blurredImage += Scalar(-50, -50, -50);
	GaussianBlur(blurredImage, blurredImage, Size(3, 3), 0, 0);
}

void ImageBinarization::createHistogram() {
	bool uniform = true;
	bool accumulate = false;

	float range[] = { 0, 256 };
	const float *histRange = range;
	calcHist(&blurredImage, 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);
}

void ImageBinarization::setThresholdValue() {
	for (int i = 0; i < histSize && threshold_value < 0.0; i++) {
		bin += cvRound(hist.at<float>(i));
		if (bin > m && threshold_value < 0.0)
			threshold_value = i;
	}
}

bool ImageBinarization::isLightingUneven() {
	return false;
}

void ImageBinarization::computeGlobalThreshold() {
	threshold(blurredImage, binarizedImage, threshold_value, max_BINARY_value, CV_THRESH_OTSU);
}

void ImageBinarization::computeLocalThreshold(int adaptiveMethod, int blockSize, int C) {
	adaptiveThreshold(blurredImage, binarizedImage, max_BINARY_value, adaptiveMethod, THRESH_BINARY, blockSize, C);
}

//Edit this to 0, if CodeFinder should only try GlobalThreshold
int ImageBinarization::getMaxThresholdMethod() {
	return 2;
}

void ImageBinarization::printThresholdMethod(int thresholdMethod) {
	cout << "Current threshold method: ";
	switch (thresholdMethod) {
	case Global:
		cout << "GlobalThreshold" << endl;
		break;

	case LocalMean:
		cout << "LocalThresholdMean" << endl;
		break;

	case LocalGaussian:
		cout << "LocalThresholdGaussian" << endl;
		break;
	}
}

//TODO: Add another Globalthresholdmethod/Localthresholdmethod or use different smoothings like medianBlur(blurredImage,blurredImage,_) 
//Also play around with blocksize, and C
Mat ImageBinarization::run(Mat image, int thresholdMethod) {

	this->image = image;
	computeSmoothing();
	createHistogram();

	printThresholdMethod(thresholdMethod);
	switch (thresholdMethod) {
	case Global:
		setThresholdValue();
		computeGlobalThreshold();
		break;

	case LocalMean:
		computeLocalThreshold(ADAPTIVE_THRESH_MEAN_C, 11, 0);
		break;

	case LocalGaussian:
		computeLocalThreshold(ADAPTIVE_THRESH_GAUSSIAN_C, 11, 0);
		break;
	}
	return binarizedImage;
}

