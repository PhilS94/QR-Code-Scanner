#include <iostream>
#include "../Header/CodeFinder.hpp"
#include "../Header/ImageBinarization.hpp"
#include <opencv2/highgui/highgui.hpp>
#include "../Header/Filesystem.hpp"

using namespace std;
using namespace cv;


/**
 * \brief Used for sorting lines along an axis.
 * \param left The point is the axis intersection and the vector describes the line.
 * \param right The point is the axis intersection and the vector describes the line.
 * \return True if right is larger than left. False otherwise.
 */
bool compareLineAlongAxis(pair<Point2f, Vec4f> left, pair<Point2f, Vec4f> right) {

	float a;
	float b;

	if (abs(left.first.x - right.first.x) > abs(left.first.y - right.first.y))
	{
		a = left.first.x;
		b = right.first.x;
	}
	else
	{
		a = left.first.y;
		b = right.first.y;
	}

	return a < b;
}

CodeFinder::CodeFinder(Mat image, bool hasCode)
	: hasCode(hasCode) {
	// TODO: Remove resizing for release version. Or keep it?
	image = image.clone();
	if (image.cols > 2000 || image.rows > 2000) {
		cout << "Resizing Image, because it is too large: " << image.rows << "x" << image.cols << ". ";

		int x = image.rows;
		int y = image.cols;
		while (x > 1500 || y > 1500)
		{
			x *= 0.9;
			y *= 0.9;
		}

		Mat resizedImage(x, y, image.type());
		resize(image, resizedImage, resizedImage.size(), 0.25, 0.25, INTER_LINEAR);
		image = resizedImage;
		cout << "New size: " << image.rows << "x" << image.cols << "." << endl;
	}

	// Saving original image.
	originalImage = image.clone();

	// Initializing default debug draw colors.
	debuggingColors.push_back(Scalar(0, 0, 255));
	debuggingColors.push_back(Scalar(0, 255, 0));
	debuggingColors.push_back(Scalar(255, 0, 0));
	debuggingColors.push_back(Scalar(255, 0, 255));
	debuggingColors.push_back(Scalar(255, 255, 0));
}

Mat CodeFinder::find() {
	Mat image = originalImage.clone();

	cout << "Converting image to binary image..." << endl;
	Mat grayscaleImage;
	cvtColor(image, grayscaleImage, CV_BGR2GRAY);

	// TODO: Get rid of ImageBinarization and put it all in this class.
	// TODO: Or get rid of transformations in this class and put them into Binarization.

	ImageBinarization binarizer;
	int thresholdMethod = -1;
	int maxThresholdMethod = binarizer.getMaxThresholdMethod();

	//Try different Thresholdmethods until one is successfull or out of thresholdmethods
	do {
		thresholdMethod++;
		binarizedImage = binarizer.run(grayscaleImage, thresholdMethod);

		cout << "	Finding all contours..." << endl;
		findAllContours();

		cout << "	Finding all finder pattern candidates..." << endl;
		findPatternContours();
		cout << "	Number of detected patterns: " << allFinderPatterns.size() << endl;

		//If less than 3 Patterns found, try different threshold method
		if (allFinderPatterns.size() < 3) {
			if (thresholdMethod < maxThresholdMethod) {
				cout << "	Try different ThresholdMethod." << endl << endl;
				continue;
			}
			cout << "Could not find any valid Patterns." << endl;
			return drawNotFound();
		}

		cout << "	Finding all edge lines for finder patterns..." << endl;;
		findPatternLines();
		cout << "	Number of detected valid patterns: " << validFinderPatterns.size() << endl;

		//If less than 3 Patterns are valid, try different threshold method
		if (validFinderPatterns.size() < 3) {
			if (thresholdMethod < maxThresholdMethod) {
				cout << "	Try different ThresholdMethod." << endl << endl;
				continue;
			}
			cout << "Could not find any valid Patterns." << endl;
			return drawNotFound();
		}

		//More than 3 valid Patterns were found. End Loop.
		break;

	} while (true);
	cout << "Successfully located FinderPatterns." << endl;

	cout << "Iterating all combinations of detected finder patterns..." << endl;
	bool isQRCode = false;
	for (int a = 0; a < validFinderPatterns.size() - 2 && !isQRCode; a++) {
		for (int b = a + 1; b < validFinderPatterns.size() - 1 && !isQRCode; b++) {
			for (int c = b + 1; c < validFinderPatterns.size() && !isQRCode; c++) {
				cout << "Examining combination (" << a + 1 << ", " << b + 1 << ", " << c + 1 << ")..." << endl;
				QRCode code;
				// TODO: Maybe don't copy here but instead use references.
				code.topLeft = validFinderPatterns[a];
				code.topRight = validFinderPatterns[b];
				code.bottomLeft = validFinderPatterns[c];

				cout << "Finding clockwise pattern order..." << endl;
				findClockwiseOrder(code);

				cout << "Finding top left corner pattern..." << endl;
				findTopLeftPattern(code);

				cout << "Finding merged lines..." << endl;
				if (!findMergedLines(code)) {
					cout << "Invalid merged lines. Combination is not a QRCode." << endl;
					continue;
				}

				cout << "Finding corners..." << endl;
				findCorners(code);

				cout << "Finding perspective transform..." << endl;
				try
				{
					findPerspectiveTransform(code);
				}
				catch(...)
				{
					cout << "Invalid perspective transform. Combination is not a QRCode." << endl;
					continue;
				}

				cout << "Finding number of modules..." << endl;
				if (!findNumberOfModules(code))
				{
					cout << "Invalid module values. Combination is not a QRCode." << endl;
					continue;
				}

				cout << "Finding resized image..." << endl;
				findResize(code);

				if (verifyQRCode(code)) {
					if(code.verifyPercentage < 85)
					{
						findAlternativeResize(code);
					}
					allCodes.push_back(code);
				}
				else
				{
					if(findAlternativeResize(code))
					{
						allCodes.push_back(code);
					}
				}
			}
		}
	}

	float verify = 0.0;
	Mat result = drawNotFound();
	for (QRCode& code : allCodes)
	{
		if (code.verifyPercentage > verify)
		{
			result = code.qrcodeImage;
			verify = code.verifyPercentage;
		}
	}

	// TODO: Consider returning all.
	return result;
}

void CodeFinder::findAllContours() {
	// Clone image, because it will be directly manipulated by findAllContours.
	Mat image = binarizedImage.clone();

	// TODO: Why is this needed?
	image /= 255;

	// TODO: Use hierarchy for finding patterns! Current way is really slow..
	findContours(image, allContours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
}

// TODO: This is incredibly slow because of the many points needed later for the segments.
bool CodeFinder::isContourInsideContour(vector<Point> in, vector<Point> out) {
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
bool CodeFinder::isTrapez(vector<Point> in) {
	vector<Point> approximatedPolygon;
	double epsilon = 0.1 * arcLength(in, true);
	approxPolyDP(in, approximatedPolygon, epsilon, true);
	bool ret = (approximatedPolygon.size() == 4);
	return ret;
}

// TODO: IMRPOVE!
void CodeFinder::findPatternContours() {
	// Thresholds for ignoring too small or too large contours.
	// TODO: Experiment with the threshold settings.
	float minArea = 8.00;
	float maxArea = 0.2 * binarizedImage.cols * binarizedImage.rows;

	for (int i = 0; i < hierarchy.size(); i++)
	{
		if (hierarchy[i][0] < 0 && hierarchy[i][1] < 0 &&
			hierarchy[i][2] < 0 && hierarchy[i][3] >= 0)
		{
			double area = contourArea(allContours[i]);
			if (minArea > area || area > maxArea) {
				continue;
			}

			int outter1 = hierarchy[i][3];
			if (hierarchy[outter1][0] < 0 && hierarchy[outter1][1] < 0 &&
				hierarchy[outter1][3] >= 0)
			{
				int outter2 = hierarchy[outter1][3];
				if (isTrapez(allContours[outter2]))
				{
					FinderPattern pattern;
					pattern.contour = allContours[outter2];
					allFinderPatterns.push_back(pattern);
				}
			}
		}
	}
}

void CodeFinder::findPatternLines() {
	for (FinderPattern &pattern : allFinderPatterns) {
		// Approximate the contour with a polygon.
		double epsilon = 0.1 * arcLength(pattern.contour, true);
		vector<Point> approximatedPolygon;
		approxPolyDP(pattern.contour, approximatedPolygon, epsilon, true);

		// Use the approximated points as cuts for segmenting the contour.
		// Find the indices of the cuts within the contour for that.
		vector<int> approxCut;
		for (Point &approxPoint : approximatedPolygon) {
			bool found = false;
			for (int index = 0; index < pattern.contour.size(); index++) {
				if (approxPoint == pattern.contour[index]) {
					approxCut.push_back(index);
					found = true;
					break;
				}
			}
			if (!found) {
				// TODO: Verify that this assumption is always true.
				// Assumption that the approximation always lies on a part of the contour did not hold true.
				break;
			}
		}
		if (approxCut.size() != 4) {
			// Assumption that the approximation is a quad did not hold true.
			continue;
		}
		sort(approxCut.begin(), approxCut.end());

		// TODO: Experiment with margin setting.
		float marginPercentage = 0.1;

		// Cut contour into segments using the approximated cutting points.
		for (int j = 0; j < 3; j++) {
			vector<Point> segment;
			int margin = (approxCut[j + 1] - approxCut[j]) * marginPercentage;
			for (int index = approxCut[j] + margin; index < approxCut[j + 1] - margin; index++) {
				segment.push_back(pattern.contour[index]);
			}
			pattern.segments.push_back(segment);
		}

		// Special case for wraping around the end of the vector.
		{
			vector<Point> segment;
			int margin = (approxCut[0] + pattern.contour.size() - approxCut[3]) * marginPercentage;
			for (int index = approxCut[3] + margin; index < approxCut[0] + pattern.contour.size() - margin; index++) {
				if (index >= pattern.contour.size())
					segment.push_back(pattern.contour[index - pattern.contour.size()]);
				else
					segment.push_back(pattern.contour[index]);
			}
			pattern.segments.push_back(segment);
		}

		// Use each cut segment for approximating a line fit.
		Vec4f line;
		for (vector<Point> segment : pattern.segments) {
			fitLine(segment, line, fitType, 0, fitReps, fitAeps);
			pattern.lines.push_back(line);
		}

		validFinderPatterns.push_back(pattern);
	}
}

void CodeFinder::findClockwiseOrder(QRCode &code) {
	// TODO: Make sure this always works!
	// Use shoelace algorithm (Gau�sche Trapezformel) to find winding order.
	int area = 0;

	Point &pa = code.topLeft.contour[0];
	Point &pb = code.topRight.contour[0];
	Point &pc = code.bottomLeft.contour[0];

	area += pa.x * (pb.y - pc.y);
	area += pb.x * (pc.y - pa.y);
	area += pc.x * (pa.y - pb.y);

	// If area is less than zero, reverse pattern order.
	if (area < 0) {
		FinderPattern temp = code.topLeft;
		code.topLeft = code.topRight;
		code.topRight = temp;
	}
}

void CodeFinder::findTopLeftPattern(QRCode &code) {
	// Compare the distances between each line for each finder pattern.
	vector<double> distanceA;
	vector<double> distanceB;
	vector<double> distanceC;

	patternPatternLineDistances(code.topLeft, code.topRight, distanceA, distanceB);
	patternPatternLineDistances(code.topLeft, code.bottomLeft, distanceA, distanceC);
	patternPatternLineDistances(code.topRight, code.bottomLeft, distanceB, distanceC);

	sort(distanceA.begin(), distanceA.end());
	sort(distanceB.begin(), distanceB.end());
	sort(distanceC.begin(), distanceC.end());

	// Every distance got essantially pushed twice so we have eight small values for
	// the top left pattern. Therefore compare the eighth value of each pattern.
	if (distanceA[7] < distanceB[7] && distanceA[7] < distanceC[7]) {
		// A is the top left pattern.
	}
	if (distanceB[7] < distanceA[7] && distanceB[7] < distanceC[7]) {
		// B is the top left pattern.
		FinderPattern temp = code.topLeft;
		code.topLeft = code.topRight;
		code.topRight = code.bottomLeft;
		code.bottomLeft = temp;
	}
	if (distanceC[7] < distanceA[7] && distanceC[7] < distanceB[7]) {
		// C is the top left pattern.
		FinderPattern temp = code.topRight;
		code.topRight = code.topLeft;
		code.topLeft = code.bottomLeft;
		code.bottomLeft = temp;
	}
}

template<typename T>
vector<T> merge(vector<T> &a, vector<T> &b) {
	vector<T> result;
	result.reserve(a.size() + b.size());
	result.insert(result.end(), a.begin(), a.end());
	result.insert(result.end(), b.begin(), b.end());
	return result;
}

bool CodeFinder::findMergedLines(QRCode &code) {
	FinderPattern &tl = code.topLeft;
	FinderPattern &tr = code.topRight;
	FinderPattern &bl = code.bottomLeft;

	// Keeps track of how the lines for the top left pattern look after merging.
	vector<Vec4f> mergedLines;
	// Keeps track of which lines already have been used.
	vector<Vec4f> usedLines;

	mergedLines.reserve(tl.lines.size());
	usedLines.reserve(tl.lines.size() * 2);

	// Merge each line of the top left pattern with one line of either top right or bottom left.
	for (int a = 0; a < tl.lines.size(); a++) {
		// Find the line with the smallest distance to the current line.
		double minDistance = lineLineDistance(tl.lines[a], tr.lines[0]);
		int index = 0;
		bool isTopRight = true;

		// Search in top right.
		for (int i = 1; i < tr.lines.size(); i++)
		{
			double currentDistance = lineLineDistance(tl.lines[a], tr.lines[i]);
			if (currentDistance < minDistance)
			{
				minDistance = currentDistance;
				index = i;
				isTopRight = true;
			}
		}
		// Search in bottom left.
		for (int i = 0; i < bl.lines.size(); i++)
		{
			double currentDistance = lineLineDistance(tl.lines[a], bl.lines[i]);
			if (currentDistance < minDistance)
			{
				minDistance = currentDistance;
				index = i;
				isTopRight = false;
			}
		}

		// Once found, merge the underlying segments and fit a new line.
		Vec4f line;
		vector<Point> mergedSegment;
		if (isTopRight) {
			mergedSegment = merge(tl.segments[a], tr.segments[index]);
			fitLine(mergedSegment, line, fitType, 0, fitReps, fitAeps);

			code.hLines.push_back(line);
			mergedLines.push_back(line);
			usedLines.push_back(tl.lines[a]);
			usedLines.push_back(tr.lines[index]);
		}
		else {
			mergedSegment = merge(tl.segments[a], bl.segments[index]);
			fitLine(mergedSegment, line, fitType, 0, fitReps, fitAeps);

			code.vLines.push_back(line);
			mergedLines.push_back(line);
			usedLines.push_back(tl.lines[a]);
			usedLines.push_back(bl.lines[index]);
		}
	}

	if (code.hLines.size() != 2 || code.vLines.size() != 2)
		return false;

	// Add all lines we have not used for merging yet.
	for (Vec4f &line : tr.lines) {
		bool used = false;
		for (Vec4f trLine : usedLines) {
			used = line == trLine;
			if (used)
				break;
		}
		if (!used)
			code.vLines.push_back(line);
	}
	for (Vec4f &line : bl.lines) {
		bool used = false;
		for (Vec4f &blLine : usedLines) {
			used = line == blLine;
			if (used)
				break;
		}
		if (!used)
			code.hLines.push_back(line);
	}

	if (code.hLines.size() != 4 || code.vLines.size() != 4)
		return false;

	// Now sort the lines within qrcode coordinate system from top left to bottom right.
	sortLinesAlongAxis(code.vLines, code.hLines[0]);
	sortLinesAlongAxis(code.hLines, code.vLines[0]);

	bool isReversed = true;
	for (Vec4f &line : mergedLines) {
		if (code.hLines[0] == line) {
			isReversed = false;
			break;
		}
	}
	if (isReversed) {
		reverse(code.hLines.begin(), code.hLines.end());
	}

	isReversed = true;
	for (Vec4f &line : mergedLines) {
		if (code.vLines[0] == line) {
			isReversed = false;
			break;
		}
	}
	if (isReversed) {
		reverse(code.vLines.begin(), code.vLines.end());
	}

	return true;
}

void CodeFinder::findCorners(QRCode &code) {
	code.corners = Mat(4, 4, DataType<Point2f>::type);
	for (int a = 0; a < code.hLines.size(); a++) {
		for (int b = 0; b < code.vLines.size(); b++) {
			Point2f result;
			if (lineIntersection(code.hLines[a], code.vLines[b], result))
				code.corners.at<Point2f>(a, b) = result;
		}
	}
}

void CodeFinder::findPerspectiveTransform(QRCode &code) {
	vector<Point2f> sourceQuad;
	sourceQuad.reserve(4);
	sourceQuad.push_back(code.corners.at<Point2f>(0, 0));
	sourceQuad.push_back(code.corners.at<Point2f>(3, 0));
	sourceQuad.push_back(code.corners.at<Point2f>(0, 3));
	sourceQuad.push_back(code.corners.at<Point2f>(3, 3));

	// Find the longest distance and use it as the target size.
	double distance = 0.0f;

	for (Point2f& p1 : sourceQuad)
	{
		for (Point2f& p2 : sourceQuad)
		{
			Point2f d = p2 - p1;
			double distanceCurrent = sqrt(d.x * d.x + d.y * d.y);
			if (distance < distanceCurrent)
				distance = distanceCurrent;
		}
	}

	distance = distance * 2;
	vector<Point2f> targetQuad;
	targetQuad.reserve(4);
	targetQuad.push_back(Point2f(0, 0));
	targetQuad.push_back(Point2f(0, distance));
	targetQuad.push_back(Point2f(distance, 0));
	targetQuad.push_back(Point2f(distance, distance));

	code.transform = getPerspectiveTransform(sourceQuad, targetQuad);

	// TODO: Experiment with different interpolation types!
	warpPerspective(binarizedImage, code.extractedImage, code.transform,
		Size(distance, distance), INTER_NEAREST);

	// TODO: The distortion of point (2, 2) can be used to more accurately extract the image.
	perspectiveTransform(code.corners, code.transformedCorners, code.transform);
}

bool CodeFinder::findNumberOfModules(QRCode& code)
{

	allCodes.push_back(code);
	showAll();

	// For each finder pattern deduce how large seven modules are.
	Point2f step21 = code.transformedCorners.at<Point2f>(1, 1);
	step21 += code.transformedCorners.at<Point2f>(1, 3) - code.transformedCorners.at<Point2f>(0, 2);
	step21 += code.transformedCorners.at<Point2f>(3, 1) - code.transformedCorners.at<Point2f>(2, 0);

	// Average the step size for a single module.
	code.gridStepSize.x = step21.x / 21;
	code.gridStepSize.y = step21.y / 21;

	// Calculate how many cells there would be with the current step size.
	double cellsX = code.extractedImage.cols / code.gridStepSize.x;
	double cellsY = code.extractedImage.rows / code.gridStepSize.y;

	cout << "Grid Step Size (x,y): " << "(" << code.gridStepSize.x << "," << code.gridStepSize.y << ")" << endl;
	cout << "Approximated module counts for X: " << cellsX << " and Y: " << cellsY << endl;
	if (cellsX < 0 || cellsY < 0 || abs(cellsX - cellsY) > 8)
	{
		return false;
	}

	// Now find the version of the qrcode by snapping to the clostest module number that is allowed.
	double totalDistance = abs(cellsX - 21) + abs(cellsY - 21);
	code.version = 2;
	while (code.version <= 40)
	{
		code.modules = 17 + 4 * code.version;
		double newTotalDistance = abs(cellsX - code.modules) + abs(cellsY - code.modules);

		if (newTotalDistance < totalDistance)
		{
			totalDistance = newTotalDistance;
			code.version++;
		}
		else
		{
			code.version--;
			break;
		}
	}

	// The result is the version, the number of modules and the step size for the qrcode.
	code.modules = 17 + 4 * code.version;
	code.gridStepSize.x = float(code.extractedImage.cols) / float(code.modules);
	code.gridStepSize.y = float(code.extractedImage.rows) / float(code.modules);
	return true;
}

void CodeFinder::findResize(QRCode& code)
{
	code.qrcodeImage = Mat(code.modules, code.modules, CV_8UC1, Scalar(255));

	for (int a = 0; a < code.modules; a++)
	{
		for (int b = 0; b < code.modules; b++)
		{
			int beginX = a * code.gridStepSize.x;
			int endX = (a + 1) * code.gridStepSize.x;

			int beginY = b * code.gridStepSize.y;
			int endY = (b + 1) * code.gridStepSize.y;

			int blackCount = 0;
			for (int x = beginX; x < endX && x < code.extractedImage.cols; x++)
			{
				for (int y = beginY; y < endY && y < code.extractedImage.rows; y++)
				{
					uint8_t pixel = code.extractedImage.at<uint8_t>(x, y);

					if (pixel == 0)
					{
						blackCount++;
					}
				}
			}
			int totalCount = (endX - beginX) * (endY - beginY);

			if (totalCount * 0.5 < blackCount)
			{
				// Set to black.
				code.qrcodeImage.at<uint8_t>(a, b) = 0;
			}
			else
			{
				// Set to white.
				code.qrcodeImage.at<uint8_t>(a, b) = 255;
			}
		}
	}
}

bool CodeFinder::findAlternativeResize(QRCode& code)
{

	float oldPercentage = code.verifyPercentage;
	int oldVersion = code.version;
	int oldModules = code.modules;
	Mat oldImage = code.qrcodeImage;

	code.version++;
	code.modules = 17 + 4 * code.version;
	code.gridStepSize.x = float(code.extractedImage.cols) / float(code.modules);
	code.gridStepSize.y = float(code.extractedImage.rows) / float(code.modules);

	findResize(code);

	verifyQRCode(code);

	if (code.verifyPercentage < oldPercentage)
	{
		code.version--;
		code.version--;
		code.modules = 17 + 4 * code.version;
		code.gridStepSize.x = float(code.extractedImage.cols) / float(code.modules);
		code.gridStepSize.y = float(code.extractedImage.rows) / float(code.modules);

		findResize(code);

		verifyQRCode(code);

		if (code.verifyPercentage < oldPercentage)
		{
			code.verifyPercentage = oldPercentage;
			code.version = oldVersion;
			code.modules = oldModules;

			findResize(code);
		}
		else
		{
			return true;
		}
	}
	else
	{
		return true;
	}

	return false;
}

// The line has to be in the same format as returned by fitLine.
double CodeFinder::pointLineDistance(Vec2f p, Vec4f line)
{
	Vec2f supportVector(line[2], line[3]);

	// Translate p so that supportVector moves into origin.
	p -= supportVector;

	// Calculate cross product between p and direction and get absolute distance.
	return abs(p[0] * line[1] - p[1] * line[0]);
}

double CodeFinder::lineLineDistance(Vec4f lineOne, Vec4f lineTwo) {
	double distOne = pointLineDistance(Vec2f(lineTwo[2], lineTwo[3]), lineOne);
	double distTwo = pointLineDistance(Vec2f(lineOne[2], lineOne[3]), lineTwo);

	return min(distOne, distTwo);
}

void CodeFinder::patternPatternLineDistances(FinderPattern &one, FinderPattern &two,
	vector<double> &distanceOne, vector<double> &distanceTwo) {
	for (Vec4f &lineOne : one.lines) {
		for (Vec4f &lineTwo : two.lines) {
			double dist = lineLineDistance(lineOne, lineTwo);
			distanceOne.push_back(dist);
			distanceTwo.push_back(dist);
		}
	}
}

bool CodeFinder::lineIntersection(Vec4f line1, Vec4f line2, Point2f &result) {
	Point2f x = Point2f(line2[2], line2[3]) - Point2f(line1[2], line1[3]);
	Point2f d1 = Point2f(line1[0], line1[1]);
	Point2f d2 = Point2f(line2[0], line2[1]);

	float cross = d1.x * d2.y - d1.y * d2.x;
	if (abs(cross) < /*EPS*/1e-8)
		return false;

	double t1 = (x.x * d2.y - x.y * d2.x) / cross;
	result = Point2f(line1[2], line1[3]) + d1 * t1;
	return true;
}

void CodeFinder::sortLinesAlongAxis(vector<Vec4f> &lines, Vec4f axis) {
	vector<pair<Point2f, Vec4f>> sortedLines;

	Point2f SV = Point2f(axis[2], axis[3]);
	Point2f Dir = Point2f(axis[0], axis[1]);
	Point2f DirOrth = Point2f(-axis[1], axis[0]);

	for (Vec4f &line : lines) {
		Point2f intersect;
		if (lineIntersection(line, axis, intersect)) {
			//intersect = intersect - SV;
			//float x = intersect.x * (-DirOrth.y) + intersect.x * Dir.y;
			sortedLines.push_back(pair<Point2f, Vec4f>(intersect, line));
		}
		else {
			sortedLines.push_back(pair<Point2f, Vec4f>(Point2f(0, 0), line));
		}
	}

	sort(sortedLines.begin(), sortedLines.end(), compareLineAlongAxis);
	lines.clear();

	for (pair<Point2f, Vec4f> &pair : sortedLines) {
		lines.push_back(pair.second);
	}
}

bool CodeFinder::verifyQRCode(QRCode &code) {
	Mat QRImage = code.qrcodeImage;
	const int cols = QRImage.cols;
	const int rows = QRImage.rows;
	int count = 0;
	int total = 0;

	//Initialize PerfectPatterns
	Mat perfectPattern = Mat::zeros(7, 7, CV_8UC1);
	for (int i = 1; i < 6; i++) {
		perfectPattern.at<uint8_t>(1, i) = 255;
		perfectPattern.at<uint8_t>(5, i) = 255;
		perfectPattern.at<uint8_t>(i, 1) = 255;
		perfectPattern.at<uint8_t>(i, 5) = 255;
	}

	//Compare to Patterns of QR
	Mat topLeftPattern = QRImage(Rect(0, 0, 7, 7));
	Mat topRightPattern = QRImage(Rect(0, cols - 7, 7, 7));
	Mat bottomLeft = QRImage(Rect(rows - 7, 0, 7, 7));


	for (int y = 0; y < 7; y++) {
		for (int x = 0; x < 7; x++) {
			total = total + 3;
			if (topLeftPattern.at<uint8_t>(x, y) == perfectPattern.at<uint8_t>(x, y))
				count++;
			if (topRightPattern.at<uint8_t>(x, y) == perfectPattern.at<uint8_t>(x, y))
				count++;
			if (bottomLeft.at<uint8_t>(x, y) == perfectPattern.at<uint8_t>(x, y))
				count++;
		}
	}

	//Initialize PerfectAligments
	int aligntmentSize = rows - 7 - 7;
	Mat perfectAlignmentPattern = Mat::zeros(1, aligntmentSize, CV_8UC1);
	for (int i = 0; i < aligntmentSize; i = i + 2) {
		perfectAlignmentPattern.at<uint8_t>(0, i) = 255;
	}

	//Compare to alignments of QR
	Mat topAlignment = QRImage(Rect(7, 6, aligntmentSize, 1));
	Mat leftAligment = QRImage(Rect(6, 7, 1, aligntmentSize));

	for (int i = 0; i < aligntmentSize; i++) {
		total = total + 2;
		if (topAlignment.at<uint8_t>(0, i) == perfectAlignmentPattern.at<uint8_t>(0, i))
			count++;
		if (leftAligment.at<uint8_t>(i, 0) == perfectAlignmentPattern.at<uint8_t>(0, i))
			count++;
	}

	float percentage = float(count) / float(total);

	// TODO: Remove for release version. Or move to a draw function.
	/*
	cout << total << endl << count << endl;
	Mat displayImage;
	resize(QRImage, displayImage, QRImage.size() * 6, 0, 0, INTER_NEAREST);
	imshow("QRImage", displayImage);
	resize(topLeftPattern, displayImage, topLeftPattern.size() * 6, 0, 0, INTER_NEAREST);
	imshow("TopLeft", displayImage);
	resize(topRightPattern, displayImage, topRightPattern.size() * 6, 0, 0, INTER_NEAREST);
	imshow("TopRight", displayImage);
	resize(bottomLeft, displayImage, bottomLeft.size() * 6, 0, 0, INTER_NEAREST);
	imshow("BottomLeft", displayImage);
	resize(topAlignment, displayImage, topAlignment.size() * 6, 0, 0, INTER_NEAREST);
	imshow("TopAlign", displayImage);
	resize(leftAligment, displayImage, leftAligment.size() * 6, 0, 0, INTER_NEAREST);
	imshow("LeftAlign", displayImage);
	waitKey(0);
	*/
	code.verifyPercentage = percentage * 100;
	cout << "Verified QRCode. Percentage is " << code.verifyPercentage << "%." << endl;
	if (code.verifyPercentage > 65)
	{
		return true;
	}
	else
	{
		cout << "Value below 65%. Combination is not a QRCode." << endl;
		return false;
	}
}

Mat CodeFinder::drawBinaryImage() {
	return binarizedImage;
}

Mat CodeFinder::drawAllContours() {
	return drawContours(allContours);
}

cv::Mat CodeFinder::drawAllContoursBinarized()
{
	Mat image = binarizedImage.clone();
	cvtColor(image, image, CV_GRAY2BGR);
	return drawContours(allContours, &image);
}

Mat CodeFinder::drawPatternContours() {
	vector<vector<Point>> patternContours;
	for (FinderPattern &pattern : validFinderPatterns) {
		patternContours.push_back(pattern.contour);
	}
	return drawContours(patternContours);
}

Mat CodeFinder::drawAllLines() {
	Mat lineImage = originalImage.clone();

	vector<vector<Point>> polygons;
	for (FinderPattern &pattern : validFinderPatterns) {
		// Approximate the contour with a polygon.
		double epsilon = 0.1 * arcLength(pattern.contour, true);
		vector<Point> approximatedPolygon;
		approxPolyDP(pattern.contour, approximatedPolygon, epsilon, true);
		polygons.push_back(approximatedPolygon);
	}

	vector<Scalar> colorsPolygons;
	colorsPolygons.push_back(Scalar(255, 255, 0));

	drawContours(polygons, &lineImage, &colorsPolygons);

	vector<Scalar> colorsLines;
	colorsLines.push_back(Scalar(0, 0, 255));

	for (FinderPattern &pattern : validFinderPatterns) {
		drawLines(pattern.lines, &lineImage, &colorsLines);
	}

	return lineImage;
}

vector<Mat> CodeFinder::drawMergedLinesAndIntersections() {
	vector<Mat> images;
	for (QRCode &code : allCodes) {
		Mat lineImage = originalImage.clone();

		drawLines(code.hLines, &lineImage);
		drawLines(code.vLines, &lineImage);

		for (int a = 0; a < code.hLines.size(); a++) {
			for (int b = 0; b < code.vLines.size(); b++) {
				Point2f intersection;
				if (lineIntersection(code.hLines[a], code.vLines[b], intersection)) {
					circle(lineImage, intersection, 3, Scalar(255, 255, 0), 2);
				}
			}
		}

		Point2f intersection;
		if (lineIntersection(code.hLines[0], code.vLines[0], intersection)) {
			circle(lineImage, intersection, 5, Scalar(255, 0, 255), 3);
		}
		if (lineIntersection(code.hLines[3], code.vLines[0], intersection)) {
			circle(lineImage, intersection, 5, Scalar(0, 0, 255), 3);
		}
		if (lineIntersection(code.hLines[0], code.vLines[3], intersection)) {
			circle(lineImage, intersection, 5, Scalar(0, 255, 0), 3);
		}
		if (lineIntersection(code.hLines[3], code.vLines[3], intersection)) {
			circle(lineImage, intersection, 5, Scalar(255, 0, 0), 3);
		}

		images.push_back(lineImage);
	}

	return images;
}

vector<Mat> CodeFinder::drawExtractedCodes() {
	vector<Mat> images;
	for (QRCode &code : allCodes) {
		images.push_back(code.extractedImage);
	}

	return images;
}

vector<Mat> CodeFinder::drawExtractedCodeGrids() {
	vector<Mat> images;
	for (QRCode &code : allCodes) {
		Mat image;
		cvtColor(code.extractedImage, image, CV_GRAY2BGR);
		for (int a = 0; a < 4; a++) {
			for (int b = 0; b < 4; b++) {
				circle(image, code.transformedCorners.at<Point2f>(a, b), 3, Scalar(0, 0, 255), 2);
			}
		}

		circle(image, code.gridStepSize, 3, Scalar(0, 255, 0), 2);

		if(code.gridStepSize.x <= 0 || code.gridStepSize.y <= 0)
		{
			continue;
		}

		vector<Vec4f> lines;
		for (int i = -7; (i * code.gridStepSize.x) < image.cols || (i * code.gridStepSize.y) < image.rows; i++)
		{
			lines.push_back(Vec4f(0, 1, code.gridStepSize.x * i, code.gridStepSize.y * i));
		}
		for (int i = -7; (i * code.gridStepSize.x) < image.cols || (i * code.gridStepSize.y) < image.rows; i++)
		{
			lines.push_back(Vec4f(1, 0, code.gridStepSize.x * i, code.gridStepSize.y * i));
		}

		vector<Scalar> colors;
		colors.push_back(Scalar(255, 0, 255));

		drawLines(lines, &image, &colors);

		images.push_back(image);
	}

	return images;
}

vector<Mat> CodeFinder::drawResized()
{
	vector<Mat> images;
	for (QRCode& code : allCodes)
	{
		if(code.qrcodeImage.data)
			images.push_back(code.qrcodeImage);
	}

	return  images;
}

Mat CodeFinder::drawAllSegments()
{
	Mat segmentImage = originalImage.clone();
	Scalar color[4];
	color[0] = Scalar(0, 0, 255);
	color[1] = Scalar(0, 255, 0);
	color[2] = Scalar(255, 0, 0);
	color[3] = Scalar(255, 255, 0);

	for (FinderPattern &pattern : validFinderPatterns) {
		for (int i = 0; i < pattern.segments.size(); i++) {
			for (Point &p : pattern.segments[i]) {
				circle(segmentImage, p, 1, color[i % 4]);
			}
		}
	}

	return segmentImage;
}

Mat CodeFinder::drawContours(vector<vector<Point>> &vecs, Mat *image, vector<Scalar> *colors) {
	Mat drawImage;
	if (!image)
		drawImage = originalImage.clone();
	else
		drawImage = *image;

	if (!colors)
		colors = &debuggingColors;

	for (int idx = 0; idx < vecs.size(); idx++) {
		cv::drawContours(drawImage, vecs, idx, (*colors)[idx % colors->size()]);
	}

	return drawImage;
}

Mat CodeFinder::drawLines(vector<Vec4f> &lines, Mat *image, vector<Scalar> *colors) {
	Mat drawImage;
	if (!image)
		drawImage = originalImage.clone();
	else
		drawImage = *image;

	if (!colors)
		colors = &debuggingColors;

	for (int idx = 0; idx < lines.size(); idx++) {
		Vec4f &line = lines[idx];
		int factor = 10000;
		float x = line[2];
		float y = line[3];
		float dx = line[0];
		float dy = line[1];
		Point p1(x - dx * factor, y - dy * factor);
		Point p2(x + dx * factor, y + dy * factor);
		cv::line(drawImage, p1, p2, (*colors)[idx % colors->size()]);
	}

	return drawImage;
}

Mat CodeFinder::drawNotFound() {
	Mat codeNotFound(1, 1, CV_8UC1);
	codeNotFound.setTo(Scalar(0));
	return codeNotFound;
}

void CodeFinder::showAll()
{
	imshow("All Contours", drawAllContours());
	imshow("Pattern Contours", drawPatternContours());
	imshow("All Segments", drawAllSegments());
	imshow("All Lines", drawAllLines());

	vector<Mat> merged = drawMergedLinesAndIntersections();
	for (int i = 0; i < merged.size(); i++) {
		imshow(string("Merged Lines And Intersections_") + to_string(i), merged[i]);
	}

	vector<Mat> extracted = drawExtractedCodes();
	for (int i = 0; i < extracted.size(); i++) {
		imshow(string("Extracted_") + to_string(i), extracted[i]);
	}

	vector<Mat> grid = drawExtractedCodeGrids();
	for (int i = 0; i < grid.size(); i++) {
		imshow(string("Extracted Grid_") + to_string(i), grid[i]);
	}

	vector<Mat> qrcodes = drawResized();
	for (int i = 0; i < qrcodes.size(); i++)
	{
		imshow(string("QRCode_") + to_string(i), qrcodes[i]);
	}
	waitKey(0);
}

void CodeFinder::saveDrawTo(const string& folder, const string&imageFilePath)
{
	FileSystem fs;
	string debugFileName;

	//**********************************************************************************************************************************************************//
	//This takes really long when the binarized image is found via localthreshold because it is "sprinkly" and has a lot of contours.
	//This means that if Codfinder::find could not find patterns via GlobalThresholding then saving this image takes incredibly long!!!!!!!!!!!!!
	Mat contour = drawAllContoursBinarized();
	debugFileName = fs.toFileName(imageFilePath) + "_1___CONTOUR___" + fs.toExtension(imageFilePath, true);
	fs.saveImage(fs.toPath(folder, debugFileName), contour);

	Mat segments = drawAllSegments();
	debugFileName = fs.toFileName(imageFilePath) + "_2___SEGMENTS___" + fs.toExtension(imageFilePath, true);
	fs.saveImage(fs.toPath(folder, debugFileName), segments);

	vector<Mat> merged = drawMergedLinesAndIntersections();
	for (int a = 0; a < merged.size(); a++) {
		debugFileName = fs.toFileName(imageFilePath) + "_3___MERGED___" + to_string(a) + fs.toExtension(imageFilePath, true);
		fs.saveImage(fs.toPath(folder, debugFileName), merged[a]);
	}

	vector<Mat> extracted = drawExtractedCodes();
	for (int a = 0; a < extracted.size(); a++) {
		debugFileName = fs.toFileName(imageFilePath) + "_4___EXTRACTED___" + to_string(a) + fs.toExtension(imageFilePath, true);
		fs.saveImage(fs.toPath(folder, debugFileName), extracted[a]);
	}

	vector<Mat> grid = drawExtractedCodeGrids();
	for (int a = 0; a < extracted.size(); a++) {
		debugFileName = fs.toFileName(imageFilePath) + "_5___GRID___" + to_string(a) + fs.toExtension(imageFilePath, true);
		fs.saveImage(fs.toPath(folder, debugFileName), grid[a]);
	}

	float verify = 0.0;
	Mat outputImage = drawNotFound();
	for (QRCode& code : allCodes)
	{
		if (code.verifyPercentage > verify)
		{
			outputImage = code.qrcodeImage;
			verify = code.verifyPercentage;
		}
	}

	if (outputImage.size().width == 1)
		return;

	debugFileName = fs.toFileName(imageFilePath) + "_6___RESULT___" + fs.toExtension(imageFilePath, true);

	fs.saveImage(fs.toPath(folder, debugFileName), outputImage);
}