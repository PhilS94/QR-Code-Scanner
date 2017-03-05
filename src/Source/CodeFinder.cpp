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
    if (left.first.x > right.first.x) {
        return true;
    }

    if (left.first.x == right.first.x) {
        if (left.first.y > right.first.y) {
            return true;
        }
    } else {
        return false;
    }

    return false;
}

CodeFinder::CodeFinder(Mat image, bool hasCode)
        : hasCode(hasCode) {
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

    // TODO: Implement repeated search if hasCode is true.
    // TODO: Get rid of ImageBinarization and put it all in this class.
    // TODO: Or get rid of transformations in this class and put them into Binarization.
    ImageBinarization binarizer;
    binarizedImage = binarizer.run(grayscaleImage);

    cout << "Finding all contours..." << endl;
    findAllContours();

    cout << "Finding all finder pattern candidates..." << endl;
    findPatternContours();
    cout << "Number of detected patterns: " << allFinderPatterns.size() << endl;
    if (allFinderPatterns.size() < 3) {
        // TODO: In case of hasCode==true don't stop here but try again starting with binarize.
        return drawNotFound();
    }

    cout << "Finding all edge lines for finder patterns..." << endl;
    findPatternLines();

    cout << "Iterating all combinations of detected finder patterns..." << endl;
    bool isQRCode = false;
    for (int a = 0; a < allFinderPatterns.size() - 2 && !isQRCode; a++) {
        for (int b = a + 1; b < allFinderPatterns.size() - 1 && !isQRCode; b++) {
            for (int c = b + 1; c < allFinderPatterns.size() && !isQRCode; c++) {
                cout << "Examining combination (" << a + 1 << ", " << b + 1 << ", " << c + 1 << ")..." << endl;
                QRCode code;
                // TODO: Maybe don't copy here but instead use references.
                code.topLeft = allFinderPatterns[a];
                code.topRight = allFinderPatterns[b];
                code.bottomLeft = allFinderPatterns[c];

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
                findPerspectiveTransform(code);

				cout << "Finding number of modules..." << endl;
				findNumberOfModules(code);

				cout << "Finding resized image..." << endl;
				findResize(code);

                // TODO: Verify that we have truly found a valid qrcode.

                allCodes.push_back(code);
            }
        }
    }

    // Return codeNotFound in case of failure to locate qrcode.
    return drawNotFound();
}

void CodeFinder::findAllContours() {
    // Clone image, because it will be directly manipulated by findAllContours.
    Mat image = binarizedImage.clone();

    // Thresholds for ignoring too small or too large contours.
    // TODO: Experiment with the threshold settings.
    float minArea = 8.00;
    float maxArea = 0.2 * image.cols * image.rows;

    // TODO: Why is this needed?
    image /= 255;

    // TODO: Use hierarchy for finding patterns! Current way is really slow..
    vector<vector<Point>> foundContours;
    vector<Vec4i> hierarchy;
    findContours(image, foundContours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

    // Remove any contours below or above the thresholds.
    allContours.reserve(foundContours.size());
    allContourAreas.reserve(foundContours.size());
    // TODO: Remove this once hierarchy is being used.
    for (int i = 0; i < foundContours.size(); i++) {
        double area = contourArea(foundContours[i]);
        if (minArea < area && area < maxArea) {
            allContours.push_back(foundContours[i]);
            allContourAreas.push_back(area);
        }
    }
    allContours.shrink_to_fit();
    allContourAreas.shrink_to_fit();
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
    // TODO: Instead of this monster, use the hierarchy function!
    for (int i = 0; i < allContours.size(); ++i) {
        for (int j = 0; j < allContours.size(); ++j) {
            if (allContourAreas.at(i) < allContourAreas.at(j)
                && isTrapez(allContours.at(j))
                && isTrapez(allContours.at(i))
                && isContourInsideContour(allContours.at(i), allContours.at(j))) {
                for (int k = 0; k < allContours.size(); ++k) {
                    if (allContourAreas.at(k) < allContourAreas.at(j)
                        && isTrapez(allContours.at(k))
                        && isContourInsideContour(allContours.at(k), allContours.at(i))) {
                        // k lies within i and i lies within j.
                        // Only save outter most contour.
                        FinderPattern pattern;
                        pattern.contour = allContours.at(j);
                        allFinderPatterns.push_back(pattern);
                    }
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
		for(int i = 1; i < tr.lines.size(); i++)
		{
			double currentDistance = lineLineDistance(tl.lines[a], tr.lines[i]);
			if(currentDistance < minDistance)
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
        } else {
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

void CodeFinder::findNumberOfModules(QRCode& code)
{
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
}

void CodeFinder::findResize(QRCode& code)
{
	code.qrcodeImage = Mat(code.modules, code.modules, CV_8UC1, Scalar(255));

	cout << "Modules: " << code.modules << endl;
	cout << "GridStepSize: " << code.gridStepSize.x << ", " << code.gridStepSize.y << endl;
	cout << "ExtractedSize: " << code.extractedImage.cols << ", " << code.extractedImage.rows << endl;
	for(int a = 0; a < code.modules; a++)
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
				for (int y = beginY; y < endY && y  < code.extractedImage.rows; y++)
				{
					uint8_t pixel = code.extractedImage.at<uint8_t>(x, y);

					if(pixel == 0)
					{
						blackCount++;
					}
				}
			}
			int totalCount = (endX - beginX) * (endY - beginY);

			if(totalCount * 0.5 < blackCount)
			{
				// Set to black.
				code.qrcodeImage.at<uint8_t>(a, b) = 0;
				//cout << "A: " << a+1 << " B: " << b+1 << " is black." << endl;
			}
			else
			{
				// Set to white.
				code.qrcodeImage.at<uint8_t>(a, b) = 255;
				//cout << "A: " << a+1 << " B: " << b+1 << " is white." << endl;
			}
		}
	}
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
    for (Vec4f &line : lines) {
        Point2f intersect;
        if (lineIntersection(line, axis, intersect)) {
            sortedLines.push_back(pair<Point2f, Vec4f>(intersect, line));
        } else {
            sortedLines.push_back(pair<Point2f, Vec4f>(Point2f(0, 0), line));
        }
    }

    sort(sortedLines.begin(), sortedLines.end(), compareLineAlongAxis);
    lines.clear();

    for (pair<Point2f, Vec4f> &pair : sortedLines) {
        lines.push_back(pair.second);
    }
}

Mat CodeFinder::drawBinaryImage() {
    return binarizedImage;
}

Mat CodeFinder::drawAllContours() {
    return drawContours(allContours);
}

Mat CodeFinder::drawPatternContours() {
    vector<vector<Point>> patternContours;
    for (FinderPattern &pattern : allFinderPatterns) {
        patternContours.push_back(pattern.contour);
    }
    return drawContours(patternContours);
}

Mat CodeFinder::drawAllLines() {
    Mat lineImage = originalImage.clone();

    vector<vector<Point>> polygons;
    for (FinderPattern &pattern : allFinderPatterns) {
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

    for (FinderPattern &pattern : allFinderPatterns) {
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

    for (FinderPattern &pattern : allFinderPatterns) {
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
