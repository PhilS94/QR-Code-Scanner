#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/tracking.hpp>

#include "../Header/Generator.hpp"
#include "../Header/Filesystem.hpp"


using namespace std;
using namespace cv;

Generator::Generator(const string source, const string dest)
	: source(source), dest(dest) {
	cout << "Reading all Images in " << source << " ..." << endl;
	workingFiles = FileSystem::allImagesAtPath(source);

	cout << "Found the following image Files: " << endl;
	for (auto it : workingFiles) {
		cout << it << endl;
	}
	cout << endl;
}

void Generator::border() {
	cout << "Generating ground truth images with border..." << endl;
	vector<string> generated;
	FileSystem fs;

	string saveFolder = fs.makeDir(dest, "01_border");


	for (auto path : workingFiles) {
		Mat image = fs.loadImage(path);
		cvtColor(image, image, CV_BGR2GRAY);

		int borderSize = image.cols * 0.25;
		Mat borderImage(image.cols + 2 * borderSize, image.rows + 2 * borderSize, image.type());
		borderImage.setTo(Scalar(255, 255, 255));
		image.copyTo(borderImage(Rect(borderSize, borderSize, image.cols, image.rows)));

		string filename = fs.toFileName(path) + "-b" + to_string(borderSize) + fs.toExtension(path, true);
		fs.saveImage(saveFolder, filename, borderImage);

		generated.push_back(fs.toPath(saveFolder, filename));
	}

	for (auto path : generated) {
		cout << path << endl;
	}
	cout << "Generated " << to_string(generated.size()) << " border images." << endl << endl;

	workingFiles = generated;
}

// MAYBE: Generate non-uniform scales.
void Generator::scale() {
	cout << "Generating scaled images from ground truth with border..." << endl;
	vector<string> generated;
	FileSystem fs;

	string saveFolder = fs.makeDir(dest, "02_scale");

	for (int i = 0; i < 3; i++) {
		auto interpolationType = INTER_NEAREST;
		string interpolationID = "-inear";

		if (i == 1) {
			interpolationType = INTER_LINEAR;
			interpolationID = "-ilinear";
		}
		else if (i == 2) {
			interpolationType = INTER_AREA;
			interpolationID = "-iarea";
		}

		// TODO: Experiment with different scale values and step sizes.
		for (float scale = 6.0f; scale < 10.1f; scale += (2.0f / 3.0f)) {
			for (auto path : workingFiles) {
				Mat image = fs.loadImage(path);
				cvtColor(image, image, CV_BGR2GRAY);
				Mat scaledImage;

				Size scaled = image.size();

				resize(image, scaledImage, Size(), scale, scale, interpolationType);

				string filename = fs.toFileName(path) + "-s" + to_string(scale).substr(0, 4) + interpolationID +
					fs.toExtension(path, true);
				fs.saveImage(saveFolder, filename, scaledImage);

				generated.push_back(fs.toPath(saveFolder, filename));
			}
		}
	}

	for (auto path : generated) {
		cout << path << endl;
	}
	cout << "Generated " << to_string(generated.size()) << " scaled images." << endl << endl;

	workingFiles = generated;
}

void Generator::rotate() {
	cout << "Generating rotated images from scaled images..." << endl;
	vector<string> generated;
	FileSystem fs;

	string saveFolder = fs.makeDir(dest, "03_rotate");

	float step_size = 36.0f;
	float max_rotation = 359.0f;

	int count = 0;
	int desiredFiles = 500;
	int estimatedFiles = workingFiles.size() * (max_rotation / step_size);
	cout << "Desired Files: " << desiredFiles << ", Estimated Files: " << estimatedFiles << endl;
	if (estimatedFiles < desiredFiles) {
		estimatedFiles = desiredFiles;
	}

	int skip = estimatedFiles / desiredFiles;
	cout << "Use every " << skip << ". image." << endl;

	for (float degree = step_size; degree < max_rotation; degree += step_size) {
		for (auto path : workingFiles) {
			count++;
			if (count % skip)
				continue;

			Mat image = fs.loadImage(path);
			cvtColor(image, image, CV_BGR2GRAY);
			Mat rotatedImage;

			Point2f image_center(image.cols / 2.0F, image.rows / 2.0F);
			Mat rot_mat = getRotationMatrix2D(image_center, degree, 1.0);
			warpAffine(image, rotatedImage, rot_mat, image.size(),1,0,cv::Scalar(255));

			string filename =
				fs.toFileName(path) + "-r" + to_string(degree).substr(0, to_string(degree).find_last_of(".")) +
				fs.toExtension(path, true);
			fs.saveImage(saveFolder, filename, rotatedImage);

			generated.push_back(fs.toPath(saveFolder, filename));
		}
	}

	for (auto path : generated) {
		cout << path << endl;
	}
	cout << "Generated " << to_string(generated.size()) << " rotated images." << endl << endl;

	workingFiles = generated;
}

/*
Apply warpPerspective by moving the topLeftCorner of the Input Image by the given step_size in X and Y Direction,
and set topRightCorner and bottomLeft correspondingly. BottomRightCorner is fixed.
*/
void Generator::perspective() {
	cout << "Generating warped images from rotated images..." << endl;
	vector<string> generated;
	FileSystem fs;

	string saveFolder = fs.makeDir(dest, "04_perspective");

	const float step_size = 0.1f;

	int count = 0;
	int desiredFiles = 1000;
	int estimatedFiles = workingFiles.size() * pow(((0.5 / step_size) - 1), 2);
	cout << "Desired Files: " << desiredFiles << ", Estimated Files: " << estimatedFiles << endl;
	if (estimatedFiles < desiredFiles) {
		estimatedFiles = desiredFiles;
	}

	int skip = estimatedFiles / desiredFiles;
	cout << "Use every " << skip << ". image." << endl;

	vector<Point2f> vecsrc;
	vector<Point2f> vecdst;
	for (auto path : workingFiles) {
		Mat image = fs.loadImage(path);
		cvtColor(image, image, CV_BGR2GRAY);
		Point2f topLeft(0, 0);
		Point2f topRight(image.cols - 1, 0);
		Point2f bottomLeft(0, image.rows - 1);
		Point2f bottomRight(image.cols - 1, image.rows - 1);

		vecsrc.clear();
		vecsrc.push_back(topLeft);
		vecsrc.push_back(topRight);
		vecsrc.push_back(bottomLeft);
		vecsrc.push_back(bottomRight);

		//Iterative through the Images topLeftQuadrant
		for (float stepY = 0; stepY < 0.5; stepY += step_size) {
			for (float stepX = 0; stepX < 0.5; stepX += step_size) {

				//If both Steps equal 0, the Transformation will be the identity -> Ignore this case
				if (stepX == 0 && stepY == 0) {
					continue;
				}

				//If StepX or StepY larger than 0.5 -> Ignore this case
				if (stepX >= 0.5 || stepY >= 0.5) {
					continue;
				}

				count++;

				//Only use every skip-th Image in order to get the desired amount of transformed Images
				if (count % skip) {
					continue;
				}

				Point2f stepPointY(0, stepY *image.rows - 1);
				Point2f vectorFromTopLeftToTopRight(topRight - stepPointY);
				Point2f stepPointX = stepX*vectorFromTopLeftToTopRight;

				Point2f warpedTopLeft = stepPointY + stepX*vectorFromTopLeftToTopRight;
				Point2f warpedTopRight = topRight - stepX*vectorFromTopLeftToTopRight;
				Point2f warpedBottomLeft = bottomLeft - stepPointY;

				vecdst.clear();
				vecdst.push_back(warpedTopLeft);
				vecdst.push_back(warpedTopRight);
				vecdst.push_back(warpedBottomLeft);
				vecdst.push_back(bottomRight);

				Mat homographyMatrix = findHomography(vecsrc, vecdst);
				Mat warpedImage = Mat(image.size(), image.type(), Scalar(255));
				warpPerspective(image, warpedImage, homographyMatrix, image.size(),1,0,Scalar(255));
				string s = "0." + to_string(cvRound(stepX * 10)) + "X" + "0." + to_string(cvRound(stepY * 10)) + "Y";
				string filename = fs.toFileName(path) + "-p" + s + fs.toExtension(path, true);
				fs.saveImage(saveFolder, filename, warpedImage);

				generated.push_back(fs.toPath(saveFolder, filename));
			}
		}
	}

	for (auto path : generated) {
		cout << path << endl;
	}
	cout << "Generated " << to_string(generated.size()) << " warped images." << endl << endl;

	workingFiles = generated;

}


void Generator::synthetic() {
	string parent = source.substr(0, source.find_last_of(separator));
	cout << "Reading all BGImages in " << parent + separator + "99_bg" << " ..." << endl;
	bgFiles = FileSystem::allImagesAtPath(parent + separator + "99_bg");

	cout << "Found the following image Files: " << endl;
	for (auto it : bgFiles) {
		cout << it << endl;
	}
	cout << endl;

	cout << "Generating syntethic images from warped images..." << endl;
	vector<string> generated;
	FileSystem fs;

	string saveFolder = fs.makeDir(dest, "05_synthetic");

	int count = 0;
	int desiredFiles = 100;
	int estimatedFiles = workingFiles.size() * bgFiles.size();
	cout << "Desired Files: " << desiredFiles << ", Estimated Files: " << estimatedFiles << endl;
	if (estimatedFiles < desiredFiles) {
		estimatedFiles = desiredFiles;
	}

	int skip = estimatedFiles / desiredFiles;
	cout << "Use every " << skip << ". image." << endl;

	for (auto path : workingFiles) {
		for (auto bgPath : bgFiles) {
			count++;

			//Only use every skip-th Image in order to get the desired amount of Images
			if (count % skip) {
				continue;
			}

			Mat qrImage = fs.loadImage(path);
			Mat bgImage = fs.loadImage(bgPath);
			Mat syntheticImage = bgImage.clone();

			int size = cvRound(0.4*min(syntheticImage.rows, syntheticImage.cols)); //Size of QRCode in syntheticImage should be approx 20%

			Mat qrResizedImage = Mat(size, size, qrImage.type());
			resize(qrImage, qrResizedImage, qrResizedImage.size(), 0, 0, INTER_LINEAR);	//TODO: Resizen, obwohl bereits bewusst verschieden resized wurde? Fragw�rdig
			
			//Randomize the position of the QRImage inside syntheticImage
			int x = rand() % (syntheticImage.cols - size);
			int y = rand() % (syntheticImage.rows - size);
			qrResizedImage.copyTo(syntheticImage(cv::Rect(x, y, size, size)));

			string filename = fs.toFileName(path) + "-syn" + fs.toFileName(bgPath) + fs.toExtension(path, true);
			fs.saveImage(saveFolder, filename, syntheticImage);

			generated.push_back(fs.toPath(saveFolder, filename));
		}
	}

	for (auto path : generated) {
		cout << path << endl;
	}
	cout << "Generated " << to_string(generated.size()) << " synthetic images." << endl << endl;

	workingFiles = generated;

}

void Generator::blur() {
}

void Generator::noise() {
}