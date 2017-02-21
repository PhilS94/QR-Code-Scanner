
#include "CodeFinder.hpp"
#include "ImageBinarization.hpp"

#include <opencv2/contrib/contrib.hpp>
#include <iostream>

using namespace std;
using namespace cv;

Mat CodeFinder::find(Mat image, bool hasCode)
{
	// TODO: Remove resizing.
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
	// Convert image to grayscale.
	Mat grayscaleImage;
	cvtColor(image, grayscaleImage, CV_BGR2GRAY);

	// TODO: Implement repeated search if hasCode is true.
	// TODO: Get rid of ImageBinarization and put it all in this class.
	// TODO: Or get rid of transformations in this class and put them into a renamed Binarization.
	// Binarize image.
	ImageBinarization binarizer;
	binarizedImage = binarizer.run(grayscaleImage);

	cout << "Finding all contours..." << endl;
	findContours();

	cout << "Finding all finder pattern candidates..." << endl;
	findFinderPatterns();

	// If there are less than three patterns repeat.

	// If there are more than three patterns try each combination until we find a qrcode and repeat if none is found.

	// If there are exactly three check if it's a qrcode and if no repeat.

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
	// TODO: Experiment with alternative approximation CV_CHAIN_APPROX_TC89_L1,CV_CHAIN_APPROX_TC89_KCOS
	vector<vector<Point>> foundContours; 
	vector<Vec4i> hierarchy;
	cv::findContours(image, foundContours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

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
	double epsilon = 0.1 * cv::arcLength(in, true);
	approxPolyDP(in, approximatedPolygon, epsilon, true);
	bool ret = (approximatedPolygon.size() == 4);
	return ret;
}

// TODO: IMRPOVE!
void CodeFinder::findFinderPatterns()
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
							finderPatternContours.push_back(contours.at(j));
							//finderPatternContours.push_back(contours.at(i));
							//finderPatternContours.push_back(contours.at(k));
						}
					}
				}
			}
		}
	}
}

cv::Mat CodeFinder::drawBinaryImage()
{
	return binarizedImage;
}

cv::Mat CodeFinder::drawContours()
{
	return draw(contours);
}

cv::Mat CodeFinder::drawFinderPatterns()
{
	return draw(finderPatternContours);
}

Point calculateMassCentres(std::vector<cv::Point> in) {

	int xCoordinates[] = { std::numeric_limits<int>::max(), std::numeric_limits<int>::min() };
	int yCoordinates[] = { std::numeric_limits<int>::max(), std::numeric_limits<int>::min() };

	//extremalwerte werden bestimmt
	for (int i = 0; i < in.size(); i++) {
		if (xCoordinates[0] > in.at(i).x) {
			xCoordinates[0] = in.at(i).x;
		}
		if (xCoordinates[1] < in.at(i).x) {
			xCoordinates[1] = in.at(i).x;
		}

		if (yCoordinates[0] > in.at(i).y) {
			yCoordinates[0] = in.at(i).y;
		}
		if (yCoordinates[1] < in.at(i).y) {
			yCoordinates[1] = in.at(i).y;
		}
	}

	int _x = (xCoordinates[0] + xCoordinates[1]) / 2;
	int _y = (yCoordinates[0] + yCoordinates[1]) / 2;

	Point point;
	point.x = _x;
	point.y = _y;

	return point;
}

double squareDistance(Point a, Point b)
{
	int x = a.x - b.x;
	int y = a.y - b.y;

	return x*x + y*y;
}

double squareDistanceToCenters(Point a, vector<Point> centers)
{
	double distance = 0.0f;
	for(Point c : centers)
	{
		distance += squareDistance(a, c);
	}
	return distance;
}

cv::Mat CodeFinder::drawApprox()
{
	vector<vector<Point>> approximatedPolygons;
	/*
	for (size_t i = 0; i < finderPatternContours.size(); i++) {
		double epsilon = 0.1 * cv::arcLength(finderPatternContours[i], true);
		vector<Point> approximatedPolygon;
		approxPolyDP(finderPatternContours[i], approximatedPolygon, epsilon, true);
		approximatedPolygons.push_back(approximatedPolygon);
	}
	*/
	
	// TODO: Ensure that at this point only ever exactly three finder patterns are used.
	if(finderPatternContours.size() == 3)
	{
		Point minX = finderPatternContours[0][0];
		Point minY = finderPatternContours[0][0];
		Point maxX = finderPatternContours[0][0];
		Point maxY = finderPatternContours[0][0];

		int count = 0;
		for (vector<Point> vec : finderPatternContours)
		{
			for (Point p : vec)
			{
				//cout << count << ": " << p.x << ", " << p.y << endl;
				count++;

				if (p.x < minX.x)
					minX = p;
				if (p.x > maxX.x)
					maxX = p;
				if (p.y < minY.y)
					minY = p;
				if (p.y > maxY.y)
					maxY = p;
			}
		}

		vector<Point> massCenters;
		for (vector<Point> vec : finderPatternContours)
		{
			massCenters.push_back(calculateMassCentres(vec));
		}

		double distMinX = squareDistanceToCenters(minX, massCenters);
		double distMinY = squareDistanceToCenters(minY, massCenters);
		double distMaxX = squareDistanceToCenters(maxX, massCenters);
		double distMaxY = squareDistanceToCenters(maxY, massCenters);

		count = 0;
		for (vector<Point> vec : finderPatternContours)
		{
			for (Point p : vec)
			{
				if (p.x == minX.x && squareDistanceToCenters(p, massCenters) > distMinX)
				{
					minX = p;
					distMinX = squareDistanceToCenters(p, massCenters);
				}
				if (p.x == maxX.x && squareDistanceToCenters(p, massCenters) > distMaxX)
				{
					maxX = p;
					distMaxX = squareDistanceToCenters(p, massCenters);
				}
				if (p.y == minY.y && squareDistanceToCenters(p, massCenters) > distMinY)
				{
					minY = p;
					distMinY = squareDistanceToCenters(p, massCenters);
				}
				if (p.y == maxY.y && squareDistanceToCenters(p, massCenters) > distMaxY)
				{
					maxY = p;
					distMaxY = squareDistanceToCenters(p, massCenters);
				}
			}
		}

		vector<Point> min;
		min.push_back(minX);
		min.push_back(minY);
		min.push_back(maxX);
		min.push_back(maxY);

		vector<Point> largestTriangle;
		double maxArea = 0.0f;
		for(int i = 0; i < min.size(); i++)
		{
			for (int j = i+1; j < min.size(); j++)
			{
				for (int k = j+1; k < min.size(); k++)
				{
					vector<Point> test;
					test.push_back(min[i]);
					test.push_back(min[j]);
					test.push_back(min[k]);
					double area = contourArea(test);
					if (area > maxArea)
					{
						largestTriangle = test;
						maxArea = area;
					}
				}
			}
		}

		approximatedPolygons.push_back(largestTriangle);

		cout << endl << endl;
		cout << "minX: " << minX.x << ", " << minX.y << endl;
		cout << "maxX: " << maxX.x << ", " << maxX.y << endl;
		cout << "minY: " << minY.x << ", " << minY.y << endl;
		cout << "maxY: " << maxY.x << ", " << maxY.y << endl;
	}

	return draw(approximatedPolygons);
}

cv::Mat CodeFinder::draw(std::vector<std::vector<cv::Point>>& vecs)
{
	cv::Mat image = originalImage.clone();
	cv::Scalar color[3];
	color[0] = cv::Scalar(0, 0, 255);
	color[1] = cv::Scalar(0, 255, 0);
	color[2] = cv::Scalar(255, 0, 0);

	for (size_t idx = 0; idx < vecs.size(); idx++) {
		cv::drawContours(image, vecs, idx, color[idx % 3]);
	}

	return image;
}
