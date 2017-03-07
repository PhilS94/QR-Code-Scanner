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
	cout << "Generating ground truth images with border.." << endl;
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

void Generator::scale(float scale) {

	if (scale < 1) {
		cout << "Could not generate scaled images, input values not correct." << endl;
		return;
	}

	cout << "Generating scaled images.." << endl;
	vector<string> generated;
	FileSystem fs;

	string saveFolder = fs.makeDir(dest, "02_scale");

	for (auto path : workingFiles) {
		Mat image = fs.loadImage(path);
		cvtColor(image, image, CV_BGR2GRAY);
		Mat scaledImage;
		Size scaled = image.size();

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

			resize(image, scaledImage, Size(), scale, scale, interpolationType);

			string s = to_string(scale).substr(0, 4) + interpolationID;
			string filename = fs.toFileName(path) + "-scale" + s + fs.toExtension(path, true);
			fs.saveImage(saveFolder, filename, scaledImage);

			generated.push_back(fs.toPath(saveFolder, filename));
		}
	}

	for (auto path : generated) {
		cout << path << endl;
	}
	cout << "Generated " << to_string(generated.size()) << " scaled images." << endl << endl;

	workingFiles = generated;
}

void Generator::rotate(int desiredFiles, float degree_step, float maxDegree) {

	if (desiredFiles < 0 || degree_step <= 0 || degree_step >= 360 || maxDegree <= 0 || maxDegree > 360) {
		cout << "Could not generate rotated images, input values not correct." << endl;
		return;
	}

	cout << "Generating rotated images.." << endl;
	vector<string> generated;
	FileSystem fs;

	string saveFolder = fs.makeDir(dest, "03_rotate");

	int estimatedFiles = workingFiles.size() * (maxDegree / degree_step);
	cout << "Desired Files: " << desiredFiles << ", Estimated Files: " << estimatedFiles << endl;
	if (desiredFiles == 0 || estimatedFiles < desiredFiles) {
		desiredFiles = estimatedFiles;
	}

	int skip = estimatedFiles / desiredFiles;
	int count = 0;

	for (float degree = degree_step; degree < maxDegree; degree += degree_step) {
		shuffle();
		for (auto path : workingFiles) {

			if (generated.size() >= desiredFiles)
				break;

			if (degree >= 360)
				break;

			count++;
			if (count % skip)
				continue;


			Mat image = fs.loadImage(path);
			cvtColor(image, image, CV_BGR2GRAY);
			Mat rotatedImage;
			Point2f image_center(image.cols / 2.0F, image.rows / 2.0F);

			Mat rot_mat = getRotationMatrix2D(image_center, degree, 1.0);
			warpAffine(image, rotatedImage, rot_mat, image.size(), 1, 0, cv::Scalar(255));

			string s = to_string(degree).substr(0, to_string(degree).find_last_of("."));
			string filename = fs.toFileName(path) + "-rot" + s + fs.toExtension(path, true);
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

void Generator::perspective(int desiredFiles, float step, float maxStep) {

	if (desiredFiles < 0 || step <= 0 || step >= 0.5 || maxStep <= 0 || maxStep >= 0.5) {
		cout << "Could not generate warped images, input values not correct." << endl;
		return;
	}

	cout << "Generating warped images.." << endl;
	vector<string> generated;
	FileSystem fs;

	string saveFolder = fs.makeDir(dest, "04_perspective");

	int estimatedFiles = workingFiles.size() * pow(((maxStep / step) + 1), 2) - 1;
	cout << "Desired Files: " << desiredFiles << ", Estimated Files: " << estimatedFiles << endl;
	if (desiredFiles == 0 || estimatedFiles < desiredFiles) {
		desiredFiles = estimatedFiles;
	}
	int skip = estimatedFiles / desiredFiles;
	int count = 0;

	vector<Point2f> vecsrc;
	vector<Point2f> vecdst;

	/*Apply warpPerspective by moving the topleftcorner of the Input Image by the given step_size in X and Y Direction, not exceeding the maxStep.
	and set toprightcorner and bottomleftcorner correspondingly. Bottomrightcorner is fixed.
	*/
	for (float stepY = 0; stepY <= maxStep; stepY += step) {
		for (float stepX = 0; stepX <= maxStep; stepX += step) {

			shuffle();
			for (auto path : workingFiles) {

				//If size of generated Images exceeds desiredFiles -> stop
				if (generated.size() >= desiredFiles)
					break;

				//If both Steps equal 0, the Transformation will be the identity -> Ignore this case
				if (stepX == 0 && stepY == 0) {
					continue;
				}

				//If StepX or StepY larger than maxStep -> Ignore this case
				if (stepX > maxStep || stepY > maxStep) {
					continue;
				}

				count++;
				if (count % skip) {
					continue;
				}

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
				warpPerspective(image, warpedImage, homographyMatrix, image.size(), 1, 0, Scalar(255));
				string s = "0." + to_string(cvRound(stepX * 10)) + "X" + "0." + to_string(cvRound(stepY * 10)) + "Y";
				string filename = fs.toFileName(path) + "-persp" + s + fs.toExtension(path, true);
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

void Generator::synthetic(int desiredFiles, float qrSize, float maxBgImageScale) {

	if (desiredFiles < 0 || qrSize <= 0 || qrSize > 1 || maxBgImageScale <= 0) {
		cout << "Could not generate synthetic images, input values not correct." << endl;
		return;
	}

	//There must be a Folder named 99_bg next to the sourceFolder of the ground_truth images.
	string bgPath = source + separator + ".." + separator + "99_bg";
	cout << "Reading all BGImages in " << bgPath << " ..." << endl;
	bgFiles = FileSystem::allImagesAtPath(bgPath);

	if (bgFiles.size() == 0) {
		cout << "Could not read background images." << endl;
		return;
	}

	cout << "Found the following image Files: " << endl;
	for (auto it : bgFiles) {
		cout << it << endl;
	}
	cout << endl;

	cout << "Generating synthetic images.." << endl;
	vector<string> generated;
	FileSystem fs;

	string saveFolder = fs.makeDir(dest, "05_synthetic");

	int estimatedFiles = workingFiles.size() * bgFiles.size();
	cout << "Desired Files: " << desiredFiles << ", Estimated Files: " << estimatedFiles << endl;
	if (desiredFiles == 0 || estimatedFiles < desiredFiles) {
		desiredFiles = estimatedFiles;
	}

	int skip = estimatedFiles / desiredFiles;
	int count = 0;

	for (auto bgPath : bgFiles) {
		shuffle();
		for (auto path : workingFiles) {
			//If size of generated Images exceeds desiredFiles -> stop
			if (generated.size() >= desiredFiles)
				break;

			count++;
			if (count % skip)
				continue;

			Mat qrImage = fs.loadImage(path);
			Mat bgImage = fs.loadImage(bgPath);
			Mat syntheticImage;

			if (bgImage.cols > maxBgImageScale || bgImage.rows > maxBgImageScale) {
				float scale;
				if (bgImage.cols > bgImage.rows) {
					scale = maxBgImageScale / bgImage.cols;
				}
				else {
					scale = maxBgImageScale / bgImage.rows;
				}
				resize(bgImage, syntheticImage, Size(), scale, scale, INTER_AREA);
			}
			else {
				syntheticImage = bgImage;
			}

			int size = cvRound(qrSize*min(syntheticImage.rows, syntheticImage.cols));		//Size of QRCode in syntheticImage should be approx qrSize%
			Mat qrResizedImage = Mat(size, size, qrImage.type());
			if (qrImage.rows > size) {
				//Shrink
				resize(qrImage, qrResizedImage, qrResizedImage.size(), 0, 0, INTER_AREA);
			}
			else {
				//Enlarge
				resize(qrImage, qrResizedImage, qrResizedImage.size(), 0, 0, INTER_LINEAR);
			}

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

void Generator::blur(int desiredFiles, int kernelSize_start, int kernelSize_step, int maxkernelSize) {

	if (desiredFiles < 0 || kernelSize_start <= 0 || kernelSize_start % 2 == 0 || kernelSize_step <= 0 || kernelSize_step % 2 != 0 || maxkernelSize <= 0) {
		cout << "Could not generate blured images, input values not correct." << endl;
		return;
	}

	cout << "Generating blured images.." << endl;
	vector<string> generated;
	FileSystem fs;

	string saveFolder = fs.makeDir(dest, "06_blured");

	int estimatedFiles = workingFiles.size() * ((maxkernelSize - kernelSize_start) / kernelSize_step + 1);
	cout << "Desired Files: " << desiredFiles << ", Estimated Files: " << estimatedFiles << endl;
	if (desiredFiles == 0 || estimatedFiles < desiredFiles) {
		desiredFiles = estimatedFiles;
	}
	int skip = estimatedFiles / desiredFiles;
	int count = 0;

	//Iterative through kernelSizes
	for (int size = kernelSize_start; size <= maxkernelSize; size += kernelSize_step) {
		shuffle();
		for (auto path : workingFiles) {
			Mat image = fs.loadImage(path);
			Mat bluredImage;

			//If size of generated Images exceeds desiredFiles -> stop
			if (generated.size() >= desiredFiles)
				break;

			count++;
			if (count % skip)
				continue;

			GaussianBlur(image, bluredImage, Size(size, size), 0, 0);
			string filename = fs.toFileName(path) + "-blur" + to_string(size) + fs.toExtension(path, true);
			fs.saveImage(saveFolder, filename, bluredImage);

			generated.push_back(fs.toPath(saveFolder, filename));
		}

	}

	for (auto path : generated) {
		cout << path << endl;
	}
	cout << "Generated " << to_string(generated.size()) << " blured images." << endl << endl;
	workingFiles = generated;
}

void Generator::noise(int desiredFiles, float stdDev_start, float stdDev_step, float maxStdDev) {

	if (desiredFiles < 0) {
		cout << "Could not generate noise images, input values not correct." << endl;
		return;
	}

	cout << "Generating noisy images.." << endl;
	vector<string> generated;
	FileSystem fs;

	string saveFolder = fs.makeDir(dest, "07_noise");

	int estimatedFiles = workingFiles.size() * ((maxStdDev - stdDev_start) / stdDev_step + 1);
	cout << "Desired Files: " << desiredFiles << ", Estimated Files: " << estimatedFiles << endl;
	if (desiredFiles == 0 || estimatedFiles < desiredFiles) {
		desiredFiles = estimatedFiles;
	}

	int skip = estimatedFiles / desiredFiles;
	int count = 0;

	for (int stddev = stdDev_start; stddev <= maxStdDev; stddev += stdDev_step) {
		shuffle();
		for (auto path : workingFiles) {
			Mat image = fs.loadImage(path);

			Mat noise(image.size(), image.type());
			Mat noiseImage;

			//If size of generated Images exceeds desiredFiles -> stop
			if (generated.size() >= desiredFiles)
				break;

			count++;
			if (count % skip)
				continue;

			randn(noise, Scalar::all(0), Scalar::all(stddev));
			addWeighted(image, 1.0, noise, 1.0, 0.0, noiseImage);


			string filename = fs.toFileName(path) + "-noise" + to_string(stddev) + fs.toExtension(path, true);
			fs.saveImage(saveFolder, filename, noiseImage);

			generated.push_back(fs.toPath(saveFolder, filename));
		}
	}

	for (auto path : generated) {
		cout << path << endl;
	}
	cout << "Generated " << to_string(generated.size()) << " noise images." << endl << endl;
	workingFiles = generated;
}

void Generator::shuffle() {
	random_shuffle(workingFiles.begin(), workingFiles.end());
}
