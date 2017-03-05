#ifndef CODE_FINDER_HPP
#define CODE_FINDER_HPP

#include <opencv2/core/core.hpp>
#include "FinderPatternModel.hpp"

class CodeFinder {
public:
	CodeFinder(cv::Mat image, bool hasCode);

	cv::Mat find();

	cv::Mat drawBinaryImage();
	cv::Mat drawAllContours();
	cv::Mat drawPatternContours();
	cv::Mat drawAllSegments();
	cv::Mat drawAllLines();
	std::vector<cv::Mat> drawMergedLinesAndIntersections();
	std::vector<cv::Mat> drawExtractedCodes();
	std::vector<cv::Mat> drawExtractedCodeGrids();
	std::vector<cv::Mat> drawResized();
	
protected:
	cv::Mat drawContours(std::vector<std::vector<cv::Point>>& vecs,
		cv::Mat* image = nullptr, std::vector<cv::Scalar>* colors = nullptr);
	cv::Mat drawLines(std::vector<cv::Vec4f>& lines,
		cv::Mat* image = nullptr, std::vector<cv::Scalar>* colors = nullptr);
	cv::Mat drawNotFound();

	void findAllContours();
	void findPatternContours();
	void findPatternLines();
	void findClockwiseOrder(QRCode& code);
	void findTopLeftPattern(QRCode& code);
	bool findMergedLines(QRCode& code);
	void findCorners(QRCode& code);
	void findPerspectiveTransform(QRCode& code);
	void findNumberOfModules(QRCode& code);
	void findResize(QRCode& code);

	bool isContourInsideContour(std::vector<cv::Point> in, std::vector<cv::Point> out);
	bool isTrapez(std::vector<cv::Point> in);

	double pointLineDistance(cv::Vec2f point, cv::Vec4f line);
	double lineLineDistance(cv::Vec4f lineOne, cv::Vec4f lineTwo);
	void patternPatternLineDistances(FinderPattern& one, FinderPattern& two,
		std::vector<double>& distanceOne, std::vector<double>& distanceTwo);
	bool lineIntersection(cv::Vec4f o1, cv::Vec4f o2, cv::Point2f &result);

	void sortLinesAlongAxis(std::vector<cv::Vec4f>& lines, cv::Vec4f axis);

private:
	bool hasCode;
	cv::Mat originalImage;
	cv::Mat binarizedImage;
	std::vector<std::vector<cv::Point>> allContours;
	std::vector<double> allContourAreas;
	std::vector<FinderPattern> allFinderPatterns;
	std::vector<QRCode> allCodes;

	// Constants used for line fitting.
	static const int fitType = CV_DIST_FAIR;
	static const int fitReps = 0.01;
	static const int fitAeps = 0.01;

	std::vector<cv::Scalar> debuggingColors;
};


#endif // CODE_FINDER_HPP
