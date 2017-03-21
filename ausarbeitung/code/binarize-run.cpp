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
