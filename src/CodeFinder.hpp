#ifndef CODE_FINDER_HPP
#define CODE_FINDER_HPP

#include <opencv2/core/core.hpp>

class CodeFinder {
public:

	cv::Mat find(cv::Mat image, bool hasCode);

	cv::Mat drawBinaryImage();
	cv::Mat drawContours();
	cv::Mat drawFinderPatterns();
	cv::Mat drawApprox();
	
protected:
	cv::Mat draw(std::vector<std::vector<cv::Point>>& vecs);

	void findContours();
	void findFinderPatterns();

	bool isContourInsideContour(std::vector<cv::Point> in, std::vector<cv::Point> out);
	bool isTrapez(std::vector<cv::Point> in);

	void minMaxDetect();

private:
	cv::Mat originalImage;
	cv::Mat binarizedImage;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<double> contourAreas;
	std::vector<std::vector<cv::Point>> finderPatternContours;
};


#endif // CODE_FINDER_HPP
