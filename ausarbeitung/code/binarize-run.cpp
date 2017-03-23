Mat ImageBinarization::run(Mat image, int thresholdMethod) {
	computeSmoothing();
	switch (thresholdMethod) {
	case Global:
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
