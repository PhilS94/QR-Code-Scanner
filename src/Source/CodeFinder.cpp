#include <opencv2/contrib/contrib.hpp>
#include <iostream>
#include "../Header/CodeFinder.hpp"
#include "../Header/ImageBinarization.hpp"
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

Mat CodeFinder::find(Mat image, bool hasCode)
{
	// TODO: Maybe ensure that all data is empty by reinitializing all structures.

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
	cout << "Number of detected patterns: " << patternContours.size() << endl;
	bool isQRCode = false;
	for(int a = 0; a < patternContours.size() - 2 && !isQRCode; a++)
	{
		for (int b = a + 1; b < patternContours.size() - 1 && !isQRCode; b++)
		{
			for (int c = b + 1; c < patternContours.size() && !isQRCode; c++)
			{
				cout << "Calculating combination (" << a << ", " << b << ", " << c << ")..." << endl;

				cout << "Finding top left corner..." << endl;
				// TODO: Identify top left corner using edge lines.
				findTopLeftPattern();

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
	contours.reserve(foundContours.size());
	contourAreas.reserve(foundContours.size());
	// TODO: If the hierarchy can be used, this will probably break it.
	for(int i = 0; i < foundContours.size(); i++)
	{
		double area = contourArea(foundContours[i]);
		if(minArea < area && area < maxArea)
		{
			contours.push_back(foundContours[i]);
			contourAreas.push_back(area);
		}
	}
	contours.shrink_to_fit();
	contourAreas.shrink_to_fit();
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
	for (int i = 0; i < contours.size(); ++i) {
		for (int j = 0; j < contours.size(); ++j) {
			if (contourAreas.at(i) < contourAreas.at(j)
				&& isContourInsideContour(contours.at(i), contours.at(j))) {
				for (int k = 0; k < contours.size(); ++k) {
					if (contourAreas.at(k) < contourAreas.at(j)
						&& isContourInsideContour(contours.at(k), contours.at(i))) {
						if (isTrapez(contours.at(j))) {
							// Only save the outter contour.
							patternContours.push_back(contours.at(j));
							//patternContours.push_back(contours.at(i));
							//patternContours.push_back(contours.at(k));
						}
					}
				}
			}
		}
	}
}

// TODO: Either remove or test if this has possible applications or performance gains.
void CodeFinder::findPatternLinesHough()
{
	Mat input(originalImage.rows, originalImage.cols, CV_8UC1, Scalar(0));

	cv::drawContours(input, patternContours, -1, Scalar(255));

	//imshow("Contour", input);

	vector<Vec2f> lines;
	double step_rho = 0.5;
	double step_theta = CV_PI / 180;
	double threshold = 20;
	HoughLines(input, lines, step_rho, step_theta, threshold, 0, 0);
	
	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0];
		float theta = lines[i][1];
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;

		Point pt1, pt2;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		line(input, pt1, pt2, Scalar(128));
	}

	//imshow("Hough", input);
	cout << "Number of lines: " << lines.size();
	waitKey(0);
}

void CodeFinder::findPatternLines()
{
	for (int i = 0; i < patternContours.size(); i++) {
		// Approximate the contour with a polygon.
		double epsilon = 0.1 * arcLength(patternContours[i], true);
		vector<Point> approximatedPolygon;
		approxPolyDP(patternContours[i], approximatedPolygon, epsilon, true);

		// Use the approximated points as cuts for segmenting the contour.
		// Find the indices of the cuts within the contour for that.
		vector<int> approxCut;
		for (Point ap : approximatedPolygon)
		{
			bool found = false;
			for (int index = 0; index < patternContours[i].size(); index++)
			{
				if (ap == patternContours[i][index])
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
				segment.push_back(patternContours[i][index]);
			}

			Vec4f line;
			fitLine(segment, line, fitType, 0, fitReps, fitAeps);
			lineSegments.push_back(segment);
			patternLines.push_back(line);
		}

		// Special case for wraping around the end of the vector.
		{
			vector<Point> segment;
			int margin = (approxCut[0] + patternContours[i].size() - approxCut[3]) * marginPercentage;
			for (int index = approxCut[3] + margin; index < approxCut[0] + patternContours[i].size() - margin; index++)
			{
				if (index >= patternContours[i].size())
					segment.push_back(patternContours[i][index - patternContours[i].size()]);
				else
					segment.push_back(patternContours[i][index]);
			}

			Vec4f line;
			fitLine(segment, line, fitType, 0, fitReps, fitAeps);
			lineSegments.push_back(segment);
			patternLines.push_back(line);
		}
	}
}

void CodeFinder::findTopLeftPattern()
{
	for(int a = 0; a < patternLines.size() - 1; a++)
	{
		for (int b = a + 1; b < patternLines.size(); b++)
		{
			// The pattern that has the four smallest distances will be the corner pattern.
			cout << "Distance between Point " << a << " and Line" << b << ": " <<
				pointToLineDistance(Vec2f(patternLines[a][2], patternLines[a][3]), patternLines[b]) << endl;
		}
	}
}

// The line has to be in the same format returned by fitLine.
double CodeFinder::pointToLineDistance(Vec2f p, Vec4f line)
{
	Vec2f supportVector(line[2], line[3]);
	// Vec2f direction(line[0], line[1]);

	// Translate p so that supportVector moves into origin.
	p -= supportVector;

	// Calculate cross product between p and direction to get distance.
	return p[0] * line[1] - p[1] * line[0];
}

Mat CodeFinder::drawBinaryImage()
{
	return binarizedImage;
}

Mat CodeFinder::drawContours()
{
	return draw(contours);
}

Mat CodeFinder::drawPatternContours()
{
	return draw(patternContours);
}

Mat CodeFinder::drawPatternLines()
{
	Mat lineImage = originalImage.clone();

	vector<vector<Point>> polygons;
	for (int i = 0; i < patternContours.size(); i++) {
		// Approximate the contour with a polygon.
		double epsilon = 0.1 * arcLength(patternContours[i], true);
		vector<Point> approximatedPolygon;
		approxPolyDP(patternContours[i], approximatedPolygon, epsilon, true);
		polygons.push_back(approximatedPolygon);
	}

	cv::drawContours(lineImage, polygons, -1, Scalar(255, 255, 0));

	for(Vec4f line : patternLines)
	{
		int factor = 10000;
		float x = line[2];
		float y = line[3];
		float dx = line[0];
		float dy = line[1];
		Point p1(x - dx * factor, y - dy * factor);
		Point p2(x + dx * factor, y + dy * factor);
		cv::line(lineImage, p1, p2, Scalar(0,0,255));
	}

	return lineImage;
}

cv::Mat CodeFinder::drawLineSegments()
{
	Mat segmentImage = originalImage.clone();
	Scalar color[4];
	color[0] = Scalar(0, 0, 255);
	color[1] = Scalar(0, 255, 0);
	color[2] = Scalar(255, 0, 0);
	color[3] = Scalar(255, 255, 0);

	for(int i = 0; i < lineSegments.size(); i++)
	{
		for(Point p : lineSegments[i])
		{
			circle(segmentImage, p, 1, color[i % 4]);
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
	color[2] = Scalar(255, 0, 0);

	for (size_t idx = 0; idx < vecs.size(); idx++) {
		cv::drawContours(image, vecs, idx, color[idx % 3]);
	}

	return image;
}
