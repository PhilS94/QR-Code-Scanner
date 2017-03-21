void ImageBinarization::computeLocalThreshold(int adaptiveMethod,
											  int blockSize, int C) {
	adaptiveThreshold(blurredImage, binarizedImage,
					  max_BINARY_value, adaptiveMethod,
					  THRESH_BINARY, blockSize, C);
}
