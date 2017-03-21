void ImageBinarization::computeGlobalThreshold() {
	threshold(blurredImage, binarizedImage,
			  threshold_value, max_BINARY_value, CV_THRESH_OTSU);
}
