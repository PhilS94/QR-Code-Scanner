#ifndef CODE_FINDER_HPP
#define CODE_FINDER_HPP

#include <opencv2/core/core.hpp>
#include "FinderPatternModel.hpp"

class CodeFinder {
public:
	CodeFinder(cv::Mat image, bool hasCode);

	cv::Mat find();

	cv::Mat drawBinaryImage();
	cv::Mat drawContours();
	cv::Mat drawPatternContours();
	cv::Mat drawPatternSegments();
	cv::Mat drawPatternLines();
	cv::Mat drawCoarseCenter();
	
protected:
	cv::Mat draw(std::vector<std::vector<cv::Point>>& vecs);

	void findContours();
	void findPatternContours();
	void findPatternLines();
	void findClockwiseOrder(FinderPattern& a, FinderPattern& b, FinderPattern& c);
	void findTopLeftPattern(FinderPattern& a, FinderPattern& b, FinderPattern& c);

	bool isContourInsideContour(std::vector<cv::Point> in, std::vector<cv::Point> out);
	bool isTrapez(std::vector<cv::Point> in);

	double pointToLineDistance(cv::Vec2f point, cv::Vec4f line);
	void patternLineDistance(FinderPattern& one, FinderPattern& two,
		std::vector<double>& distanceOne, std::vector<double>& distanceTwo);

private:
	bool hasCode;
	cv::Mat originalImage;
	cv::Mat binarizedImage;
	std::vector<std::vector<cv::Point>> allContours;
	std::vector<double> allContourAreas;
	std::vector<FinderPattern> finderPatterns;
};


#endif // CODE_FINDER_HPP
