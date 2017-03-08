#ifndef CODE_FINDER_HPP
#define CODE_FINDER_HPP

#include <opencv2/core/core.hpp>
#include "FinderPatternModel.hpp"


/**
 * \brief Takes an image and analyzes it trying to find a QRCode.
 * NOTE: Function documentation in .cpp only.
 */
class CodeFinder {
public:
    CodeFinder(cv::Mat image, bool hasCode);

    cv::Mat find();

	static cv::Mat drawNotFound();
	
	cv::Mat drawBinaryImage();
	cv::Mat drawAllContours();
	cv::Mat drawAllContoursBinarized();
	cv::Mat drawPatternContours();
	cv::Mat drawAllSegments();
	cv::Mat drawAllLines();
	std::vector<cv::Mat> drawMergedLinesAndIntersections();
	std::vector<cv::Mat> drawExtractedCodes();
	std::vector<cv::Mat> drawExtractedCodeGrids();
	std::vector<cv::Mat> drawResized();

	void showAll();
	void saveDrawTo(const std::string& folder, const std::string& imageFilePath);

protected:
    cv::Mat drawContours(std::vector<std::vector<cv::Point>> &vecs,
                         cv::Mat *image = nullptr, std::vector<cv::Scalar> *colors = nullptr);

    cv::Mat drawLines(std::vector<cv::Vec4f> &lines,
                      cv::Mat *image = nullptr, std::vector<cv::Scalar> *colors = nullptr);

	void findAllContours();
	void findPatternContours();
	void findPatternLines();
	void findClockwiseOrder(QRCode& code);
	void findTopLeftPattern(QRCode& code);
	bool findMergedLines(QRCode& code);
	void findCorners(QRCode& code);
	void findPerspectiveTransform(QRCode& code);
	bool findNumberOfModules(QRCode& code);
	void findResize(QRCode& code);
	bool findAlternativeResize(QRCode& code);
	bool verifyQRCode(QRCode& code);

    bool isTrapez(std::vector<cv::Point> in);

    double pointLineDistance(cv::Vec2f point, cv::Vec4f line);

    double lineLineDistance(cv::Vec4f lineOne, cv::Vec4f lineTwo);

    void patternPatternLineDistances(FinderPattern &one, FinderPattern &two,
                                     std::vector<double> &distanceOne, std::vector<double> &distanceTwo);

    bool lineIntersection(cv::Vec4f o1, cv::Vec4f o2, cv::Point2f &result);

    void sortLinesAlongAxis(std::vector<cv::Vec4f> &lines, cv::Vec4f axis);


private:
	/* Not used. */
    bool hasCode;

	/* Original image passed for searching. */
    cv::Mat originalImage;

	/* Image after going through binarization. */
    cv::Mat binarizedImage;

	/* All contours found in the current binarized image.*/
    std::vector<std::vector<cv::Point>> allContours;

	/* Hierarchy describing the realtionship of all contours. */
	std::vector<cv::Vec4i> hierarchy;

	/* All contours identified as possible finder patterns. */
	std::vector<FinderPattern> allFinderPatterns;

	/* All finder patterns that passed the selection. */
	std::vector<FinderPattern> validFinderPatterns;

	/* All evaluated combinations of finder patterns that yielded a result. */
    std::vector<QRCode> allCodes;

    // Constants used for line fitting.
    static const int fitType = CV_DIST_FAIR;
    static const int fitReps = 0.01;
    static const int fitAeps = 0.01;

	/* Array with colors for debugging. */
    std::vector<cv::Scalar> debuggingColors;
};


#endif // CODE_FINDER_HPP
