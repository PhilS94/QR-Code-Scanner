#ifndef CODE_FINDER_HPP
#define CODE_FINDER_HPP

#include <opencv2/core/core.hpp>

class CodeFinder {
public:

	cv::Mat find(cv::Mat image, bool hasCode);

	cv::Mat drawBinaryImage();
	cv::Mat drawContours();
	cv::Mat drawPatternContours();
	cv::Mat drawPatternLines();
	cv::Mat drawLineSegments();
	
protected:
	cv::Mat draw(std::vector<std::vector<cv::Point>>& vecs);

	void findContours();
	void findPatternContours();
	void findPatternLines();
	void findTopLeftPattern();

	bool isContourInsideContour(std::vector<cv::Point> in, std::vector<cv::Point> out);
	bool isTrapez(std::vector<cv::Point> in);

	double pointToLineDistance(cv::Vec2f point, cv::Vec4f line);

	void findPatternLinesHough();

private:
	cv::Mat originalImage;
	cv::Mat binarizedImage;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<double> contourAreas;
	std::vector<std::vector<cv::Point>> patternContours;
	std::vector<std::vector<cv::Point>> lineSegments;
	std::vector<cv::Vec4f> patternLines;
	std::vector<cv::Vec4f> patternLinesHough;
};


#endif // CODE_FINDER_HPP
