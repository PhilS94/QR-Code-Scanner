#include <opencv2/contrib/contrib.hpp>
#include <iostream>
#include "../Header/CodeFinder.hpp"
#include "../Header/ImageBinarization.hpp"
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

CodeFinder::CodeFinder(cv::Mat image, bool hasCode)
	: hasCode(hasCode)
{
	// TODO: Remove resizing for release version.
	image = image.clone();
	if (image.cols > 2000 || image.rows > 2000) {
		cout << "Resizing Image, because it is too large: " << image.rows << "x" << image.cols << ". ";
		Mat resizedImage(0.25 * image.rows, 0.25 * image.cols, image.type());
		resize(image, resizedImage, resizedImage.size(), 0.25, 0.25, INTER_LINEAR);
		image = resizedImage;
		cout << "New size: " << image.rows << "x" << image.cols << "." << endl;
	}

	// Saving original image.
	originalImage = image.clone();
}

Mat CodeFinder::find()
{
	Mat image = originalImage.clone();

	cout << "Converting image to binary image..." << endl;
	Mat grayscaleImage;
	cvtColor(image, grayscaleImage, CV_BGR2GRAY);

	// TODO: Implement repeated search if hasCode is true.
	// TODO: Get rid of ImageBinarization and put it all in this class.
	// TODO: Or get rid of transformations in this class and put them into a renamed Binarization.
	ImageBinarization binarizer;
	binarizedImage = binarizer.run(grayscaleImage);

	cout << "Finding all contours..." << endl;
	findContours();

	cout << "Finding all finder pattern candidates..." << endl;
	findPatternContours();
	// TODO: At this point, if there are less than three patterns, try again starting with binarization if hasCode is true or abort otherwise.

	cout << "Finding all edge lines for finder patterns..." << endl;
	findPatternLines();

	cout << "Iterating all combinations of detected finder patterns..." << endl;
	cout << "Number of detected patterns: " << finderPatterns.size() << endl;
	bool isQRCode = false;
	for(int a = 0; a < finderPatterns.size() - 2 && !isQRCode; a++)
	{
		for (int b = a + 1; b < finderPatterns.size() - 1 && !isQRCode; b++)
		{
			for (int c = b + 1; c < finderPatterns.size() && !isQRCode; c++)
			{
				cout << "Calculating combination (" << a << ", " << b << ", " << c << ")..." << endl;

				FinderPattern fpTopLeft = finderPatterns[a];
				FinderPattern fpTopRight = finderPatterns[b];
				FinderPattern fpBottomLeft = finderPatterns[c];

				cout << "Finding clockwise pattern order..." << endl;
				findClockwiseOrder(fpTopLeft, fpTopRight, fpBottomLeft);

				cout << "Finding top left corner pattern..." << endl;
				findTopLeftPattern(fpTopLeft, fpTopRight, fpBottomLeft);

				// TODO: Calculate for each line the divergence and merge near identical lines by recalcualting a new line with the help of both underlying segments.
				// It appears like edge line detection is stable. => Line 1 of pattern 1 can only be a duplicate of line 1 of pattern 2 and so forth.

				// TODO: Find the four corners using the edge line intersections.
				// In case of obviously degenerate lines abort.

				// TODO: Use perspective transform to extract the qrcode.

				// TODO: Attempt calculation of true size of the qrcode.

				// TODO: Resize to true size of qrcode.

				// TODO: Verify that we have truly found a valid qrcode.
			}
		}
	}

	// Return codeNotFound in case of failure to locate qrcode.
	Mat codeNotFound(1, 1, image.type());
	codeNotFound.setTo(Scalar(0));
	return codeNotFound;
}

void CodeFinder::findContours()
{
	// Clone image, because it will be directly manipulated by findContours.
	// TODO: Maybe don't do this because it wont be used again anway.
	Mat image = binarizedImage.clone();

	// Thresholds for ignoring too small or too large contours.
	// TODO: Experiment with the threshold settings.
	float minArea = 8.00;
	float maxArea = 0.2 * image.cols * image.rows;

	// TODO: Why is this needed?
	image /= 255;

	// TODO: Can we use the hierarchy for identifying finder patterns? If not replace RETR_TREE with RETR_LIST.
	vector<vector<Point>> foundContours; 
	vector<Vec4i> hierarchy;
	cv::findContours(image, foundContours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

	// Remove any contours below or above the thresholds.
	allContours.reserve(foundContours.size());
	allContourAreas.reserve(foundContours.size());
	// TODO: If the hierarchy can be used, this will probably break it.
	for(int i = 0; i < foundContours.size(); i++)
	{
		double area = contourArea(foundContours[i]);
		if(minArea < area && area < maxArea)
		{
			allContours.push_back(foundContours[i]);
			allContourAreas.push_back(area);
		}
	}
	allContours.shrink_to_fit();
	allContourAreas.shrink_to_fit();
}
// TODO: Imrpove inside contour detection.
bool CodeFinder::isContourInsideContour(vector<Point> in, vector<Point> out)
{
	if (in.size() > 0 && out.size() > 0) {
		for (int i = 0; i < in.size(); i++) {
			if (pointPolygonTest(out, in[i], false) <= 0)
				return false;
		}
		return true;
	}
	return false;
}

// TODO: Imrpove Trapez detection function.
bool CodeFinder::isTrapez(vector<Point> in)
{
	vector<Point> approximatedPolygon;
	double epsilon = 0.1 * arcLength(in, true);
	approxPolyDP(in, approximatedPolygon, epsilon, true);
	bool ret = (approximatedPolygon.size() == 4);
	return ret;
}

// TODO: IMRPOVE!
void CodeFinder::findPatternContours()
{
	for (int i = 0; i < allContours.size(); ++i) {
		for (int j = 0; j < allContours.size(); ++j) {
			if (allContourAreas.at(i) < allContourAreas.at(j)
				&& isContourInsideContour(allContours.at(i), allContours.at(j))) {
				for (int k = 0; k < allContours.size(); ++k) {
					if (allContourAreas.at(k) < allContourAreas.at(j)
						&& isContourInsideContour(allContours.at(k), allContours.at(i))) {
						if (isTrapez(allContours.at(j))) {
							// k lies within i and i lies within j.
							// Only save outter most contour.
							FinderPattern pattern;
							pattern.contour = allContours.at(j);
							finderPatterns.push_back(pattern);
						}
					}
				}
			}
		}
	}
}

void CodeFinder::findPatternLines()
{
	for (FinderPattern& pattern : finderPatterns) {
		// Approximate the contour with a polygon.
		double epsilon = 0.1 * arcLength(pattern.contour, true);
		vector<Point> approximatedPolygon;
		approxPolyDP(pattern.contour, approximatedPolygon, epsilon, true);

		// Use the approximated points as cuts for segmenting the contour.
		// Find the indices of the cuts within the contour for that.
		vector<int> approxCut;
		for (Point& approxPoint : approximatedPolygon)
		{
			bool found = false;
			for (int index = 0; index < pattern.contour.size(); index++)
			{
				if (approxPoint == pattern.contour[index])
				{
					approxCut.push_back(index);
					found = true;
					break;
				}
			}
			if (!found)
			{
				// TODO: Verify that this assumption is always true.
				throw exception(); // Assumption that the approximation always lies on a part of the contour did not hold true.
			}
		}

		if (approxCut.size() != 4)
		{
			// TODO: Maybe do something so this doesn't kill the program.
			throw exception(); // Assumption that the approximation is a quad did not hold true.
		}
		sort(approxCut.begin(), approxCut.end());

		int fitType = CV_DIST_FAIR;
		int fitReps = 0.01;
		int fitAeps = 0.01;

		float marginPercentage = 0.1;

		// Use each cut segment for approximating a line fit.
		for (int j = 0; j < 3; j++)
		{
			vector<Point> segment;
			int margin = (approxCut[j + 1] - approxCut[j]) * marginPercentage;
			for (int index = approxCut[j] + margin; index < approxCut[j + 1] - margin; index++)
			{
				segment.push_back(pattern.contour[index]);
			}

			Vec4f line;
			fitLine(segment, line, fitType, 0, fitReps, fitAeps);
			pattern.segments.push_back(segment);
			pattern.lines.push_back(line);
		}

		// Special case for wraping around the end of the vector.
		{
			vector<Point> segment;
			int margin = (approxCut[0] + pattern.contour.size() - approxCut[3]) * marginPercentage;
			for (int index = approxCut[3] + margin; index < approxCut[0] + pattern.contour.size() - margin; index++)
			{
				if (index >= pattern.contour.size())
					segment.push_back(pattern.contour[index - pattern.contour.size()]);
				else
					segment.push_back(pattern.contour[index]);
			}

			Vec4f line;
			fitLine(segment, line, fitType, 0, fitReps, fitAeps);
			pattern.segments.push_back(segment);
			pattern.lines.push_back(line);
		}
	}
}

void CodeFinder::findClockwiseOrder(FinderPattern& a, FinderPattern& b, FinderPattern& c)
{
	// TODO: Make sure this always works!
	// Use shoelace algorithm (Gauﬂsche Trapezformel) to find winding order.
	int area = 0;

	Point pa = a.contour[0];
	Point pb = b.contour[0];
	Point pc = c.contour[0];

	area += pa.x * (pb.y - pc.y);
	area += pb.x * (pc.y - pa.y);
	area += pc.x * (pa.y - pb.y);

	// If area is less than zero, reverse pattern order.
	if (area < 0)
	{
		FinderPattern temp = a;
		a = b;
		b = temp;
	}
}

void CodeFinder::findTopLeftPattern(FinderPattern& a, FinderPattern& b, FinderPattern& c)
{
	// Compare the distances between each line for each finder pattern.
	vector<double> distanceA;
	vector<double> distanceB;
	vector<double> distanceC;

	patternLineDistance(a, b, distanceA, distanceB);
	patternLineDistance(a, c, distanceA, distanceC);
	patternLineDistance(b, c, distanceB, distanceC);

	sort(distanceA.begin(), distanceA.end());
	sort(distanceB.begin(), distanceB.end());
	sort(distanceC.begin(), distanceC.end());

	// Every distance got essantially pushed twice so we have eight small values for
	// the top left pattern. Therefore compare the eighth value of each pattern.
	Mat topLeftImage;
	vector<vector<Point>> contours;
	if(distanceA[7] < distanceB[7] && distanceA[7] < distanceC[7])
	{
		// A is the top left pattern.
		contours.push_back(a.contour);
		contours.push_back(b.contour);
		contours.push_back(c.contour);
		topLeftImage = draw(contours);
	}
	if (distanceB[7] < distanceA[7] && distanceB[7] < distanceC[7])
	{
		// B is the top left pattern.
		contours.push_back(b.contour);
		contours.push_back(c.contour);
		contours.push_back(a.contour);
		topLeftImage = draw(contours);
		FinderPattern temp = a;
		a = b;
		b = c;
		c = temp;
	}
	if (distanceC[7] < distanceA[7] && distanceC[7] < distanceB[7])
	{
		// C is the top left pattern.
		contours.push_back(c.contour);
		contours.push_back(a.contour);
		contours.push_back(b.contour);
		topLeftImage = draw(contours);
		FinderPattern temp = b;
		a = c;
		b = a;
		c = temp;
	}
	imshow("Top Left", topLeftImage);
}

// The line has to be in the same format as returned by fitLine.
double CodeFinder::pointToLineDistance(Vec2f p, Vec4f line)
{
	Vec2f supportVector(line[2], line[3]);
	// Vec2f direction(line[0], line[1]);

	// Translate p so that supportVector moves into origin.
	p -= supportVector;

	// Calculate cross product between p and direction and get absolute distance.
	return abs(p[0] * line[1] - p[1] * line[0]);
}

void CodeFinder::patternLineDistance(FinderPattern& one, FinderPattern& two,
	vector<double>& distanceOne, vector<double>& distanceTwo)
{
	for (Vec4f lineOne : one.lines)
	{
		for (Vec4f lineTwo : two.lines)
		{
			// TODO: Is it enough only to look in one direction?
			double distOne = pointToLineDistance(Vec2f(lineTwo[2], lineTwo[3]), lineOne);
			double distTwo = pointToLineDistance(Vec2f(lineOne[2], lineOne[3]), lineTwo);
			distanceOne.push_back(distOne);
			distanceOne.push_back(distTwo);
			distanceTwo.push_back(distOne);
			distanceTwo.push_back(distTwo);
		}
	}
}

Mat CodeFinder::drawBinaryImage()
{
	return binarizedImage;
}

Mat CodeFinder::drawContours()
{
	return draw(allContours);
}

Mat CodeFinder::drawPatternContours()
{
	std::vector<std::vector<cv::Point>> patternContours;
	for(FinderPattern& pattern : finderPatterns)
	{
		patternContours.push_back(pattern.contour);
	}
	return draw(patternContours);
}

Mat CodeFinder::drawPatternLines()
{
	Mat lineImage = originalImage.clone();

	vector<vector<Point>> polygons;
	for (FinderPattern& pattern : finderPatterns) {
		// Approximate the contour with a polygon.
		double epsilon = 0.1 * arcLength(pattern.contour, true);
		vector<Point> approximatedPolygon;
		approxPolyDP(pattern.contour, approximatedPolygon, epsilon, true);
		polygons.push_back(approximatedPolygon);
	}

	cv::drawContours(lineImage, polygons, -1, Scalar(255, 255, 0));

	for (FinderPattern& pattern : finderPatterns) {
		for (Vec4f& line : pattern.lines)
		{
			int factor = 10000;
			float x = line[2];
			float y = line[3];
			float dx = line[0];
			float dy = line[1];
			Point p1(x - dx * factor, y - dy * factor);
			Point p2(x + dx * factor, y + dy * factor);
			cv::line(lineImage, p1, p2, Scalar(0, 0, 255));
		}
	}

	return lineImage;
}

cv::Mat CodeFinder::drawCoarseCenter()
{
	Mat coarseImage = originalImage.clone();

	for(FinderPattern& pattern : finderPatterns)
	{
		circle(coarseImage, pattern.contour[0], 5, Scalar(0, 0, 255), 3);
	}

	for (int a = 0; a < finderPatterns.size() - 2; a++)
	{
		for (int b = a + 1; b < finderPatterns.size() - 1; b++)
		{
			for (int c = b + 1; c < finderPatterns.size(); c++)
			{
				FinderPattern& fpA = finderPatterns[a];
				FinderPattern& fpB = finderPatterns[b];
				FinderPattern& fpC = finderPatterns[c];

				Point2f coarseCenter = (fpA.contour[0] + fpB.contour[0] + fpC.contour[0]);
				coarseCenter.x /= 3;
				coarseCenter.y /= 3;

				circle(coarseImage, coarseCenter, 5, Scalar(255, 0, 255), 3);
			}
		}
	}

	return coarseImage;
}

cv::Mat CodeFinder::drawPatternSegments()
{
	Mat segmentImage = originalImage.clone();
	Scalar color[4];
	color[0] = Scalar(0, 0, 255);
	color[1] = Scalar(0, 255, 0);
	color[2] = Scalar(255, 0, 0);
	color[3] = Scalar(255, 255, 0);

	for(FinderPattern& pattern : finderPatterns)
	{
		for(int i = 0; i < pattern.segments.size(); i++)
		{
			for (Point& p : pattern.segments[i])
			{
				circle(segmentImage, p, 1, color[i % 4]);
			}
		}
	}

	return segmentImage;
}

Mat CodeFinder::draw(vector<vector<Point>>& vecs)
{
	Mat image = originalImage.clone();
	Scalar color[3];
	color[0] = Scalar(0, 0, 255);
	color[1] = Scalar(0, 255, 0);
	color[2] = Scalar(255, 0, 255);

	for (size_t idx = 0; idx < vecs.size(); idx++) {
		cv::drawContours(image, vecs, idx, color[idx % 3]);
	}

	return image;
}
