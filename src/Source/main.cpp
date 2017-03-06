#include <iostream>
#include <opencv2/opencv.hpp>
#include "../Header/ImageBinarization.hpp"
#include "../Header/Filesystem.hpp"
#include "../Header/Generator.hpp"
#include "../Header/CodeFinder.hpp"

using namespace std;
using namespace cv;

void cameraMode();

void folderMode(const string &path);

void evaluationMode(const string &source, const string &dest);

void generateMode(const string &source, const string &dest);

float evaluate(const string &source, const Mat &outputImage);


void printLogo() {
	cout << "+---------------------------------------------------------------------------+\n"
		"|     _______  _______    _______  _______  ______   _______                |\n"
		"|    (  ___  )(  ____ )  (  ____ \\(  ___  )(  __  \\ (  ____ \\               |\n"
		"|    | (   ) || (    )|  | (    \\/| (   ) || (  \\  )| (    \\/               |\n"
		"|    | |   | || (____)|  | |      | |   | || |   ) || (__                   |\n"
		"|    | |   | ||     __)  | |      | |   | || |   | ||  __)                  |\n"
		"|    | | /\\| || (\\ (     | |      | |   | || |   ) || (                     |\n"
		"|    | (_\\ \\ || ) \\ \\__  | (____/\\| (___) || (__/  )| (____/\\               |\n"
		"|    (____\\/_)|/   \\__/  (_______/(_______)(______/ (_______/               |\n"
		"+---------------------------------------------------------------------------+" << endl;
}


void printUsage() {
	cout << "|                                                                           |\n"
		"| Please enter 0, 1 or 2 path values to start one of the following modes:   |\n"
		"|                                                                           |\n"
		"|     - 0 path values: Camera Mode.                                         |\n"
		"|       Attempt to open a camera feed and continously search for qrcodes.   |\n"
		"|                                                                           |\n"
		"|     - 1 path values: Folder Scan Mode.                                    |\n"
		"|       Scan all images in the folder for qrcodes and save the detection    |\n"
		"|       results into a subfolder.                                           |\n"
		"|                                                                           |\n"
		"|     - 2 path values: Evaluation Mode.                                     |\n"
		"|       Read image stored at input-path and save the detection result       |\n"
		"|       to output-path.                                                     |\n"
		"|                                                                           |\n"
		"|     - 2 path values and \"-generate\": Generate Mode.                       |\n"
		"|       Read images stored at ground-truth-path and generate syntethic      |\n"
		"|       database images at output-path                                      |\n"
		"|                                                                           |\n"
		"| Usage: <main>                                                             |\n"
		"| Usage: <main> [<folder-path>]                                             |\n"
		"| Usage: <main> [<input-path>] [<output-path>]                              |\n"
		"| Usage: <main> [-generate] [<ground-truth-path>] [<output-path>]           |\n"
		"+---------------------------------------------------------------------------+" << endl;
}


int main(int argc, const char *argv[]) {
	cout << "Path to executable: " << argv[0] << endl;
	printLogo();

	if (argc == 1) {
		cout << "Starting Camera Mode..." << endl;
		cameraMode();
	}
	else if (argc == 2) {
		cout << "Starting Folder Scan Mode..." << endl;
		cout << argv[1] << endl;
		folderMode(argv[1]);
	}
	else if (argc == 3) {
		cout << "Starting Evaluation Mode..." << endl;
		evaluationMode(argv[1], argv[2]);
	}
	else if (argc == 4 && string(argv[1]) == "-generate") {
		cout << "Starting Generate Mode..." << endl;
		generateMode(argv[2], argv[3]);
	}
	else {
		printUsage();
		cout << endl << endl << "Failed to select mode. Printing Arguments:" << endl << endl;
		for (int i = 0; i < argc; i++) {
			cout << argv[i] << endl;
		}
	}

#ifdef _WIN32
	system("pause");
#else
	waitKey(0);
#endif

	return 0;
}


void cameraMode() {
	VideoCapture cap(0); // open the default camera
	if (!cap.isOpened())  // check if we succeeded
	{
		cout << "Could not open camera." << endl;
		return;
	}

	namedWindow("Video", 1);
	while (1) {
		Mat frame;
		cap >> frame;         // get a new frame from camera
		imshow("Video", frame);

		CodeFinder codeFinder(frame, false);
		Mat outputImage = codeFinder.find();

		vector<Mat> merged = codeFinder.drawMergedLinesAndIntersections();
		for (int i = 0; i < merged.size(); i++) {
			imshow(string("Merged Lines And Intersections_") + to_string(i), merged[i]);
		}

		vector<Mat> extracted = codeFinder.drawExtractedCodes();
		for (int i = 0; i < extracted.size(); i++) {
			imshow(string("Extracted_") + to_string(i), extracted[i]);
		}

		// Press 'c' to escape
		if (waitKey(30) == 'c') break;
	}
}

void folderMode(const string &source) {
	cout << "Reading all Files in " << source << " ..." << endl;

	vector<string> imageFiles = FileSystem::allImagesAtPath(source);

	cout << "Found the following valid Files: " << endl;
	for (auto it = imageFiles.begin(); it != imageFiles.end(); ++it) {
		cout << *it << endl;
	}
	cout << endl;

	char confirm;
	cout << "Do you want to read all these files and test QR-Code Algorithm? If yes press 'y':  ";
	cin >> confirm;

    if (confirm == 'y') {
        cout << "Now start iterating through all Images.." << endl;
		float evaluateAverage = 0;
		float detected = 0;
		float evaluateCount = 0;
		FileSystem fs;
		string debugFolder = fs.makeDir(source, "ScanResult");
        for (int i = 0; i < imageFiles.size(); i++) {
            cout << "Processing file <" << i << "> of <" << imageFiles.size() << ">." << endl;
            cout << "Path: " << imageFiles[i] << endl;
            Mat image = fs.loadImage(imageFiles[i]);
			CodeFinder codeFinder(image, false);
            Mat outputImage = codeFinder.find();

			// Code for debugging and evaluating.
			if(outputImage.size().width != 1)
			{
				float result = evaluate(imageFiles[i], outputImage);
				if(result != -1)
				{
					evaluateAverage += result;
					evaluateCount++;
				}
				detected++;
			}
			cout << endl;

			codeFinder.saveDrawTo(debugFolder, imageFiles[i]);
        }

		cout << endl;
		cout << "Finished iterating through all Images." << endl;
		cout << "#Images: " << imageFiles.size() << " #QRCodes: " << detected <<
			" AverageQuality: " << evaluateAverage / evaluateCount << " Correct Size: " << evaluateCount << endl;
	}
	else {
		cout << endl << "Aborted." << endl << endl;
	}
}

void evaluationMode(const string &source, const string &dest) {
	Mat inputImage = FileSystem::loadImage(source);

	CodeFinder codeFinder(inputImage, true);
	Mat outputImage = codeFinder.find();

	evaluate(source, outputImage);
	
	FileSystem::makeDir(FileSystem::toFolderPath(dest));
	//FileSystem::saveImage(dest, outputImage);

	codeFinder.showAll();
}


void generateMode(const string &source, const string &dest) {
	cout << "Source     : " << source << endl;
	cout << "Destination: " << dest << endl;

	Generator gen(source, dest);
	gen.border();
	gen.scale();
	gen.rotate();
	gen.perspective();
	gen.synthetic();
}

float evaluate(const string &source, const Mat &outputImage) {

	cout << "Source: " << source << endl;

	float equality;
	string sourceFilename = FileSystem::toFileName(source);
	int secondDash = sourceFilename.find_first_of("-", sourceFilename.find_first_of("-") + 1);
	string groundtruthFilename = sourceFilename.substr(0, secondDash);

	string groundTruthImagePath = FileSystem::toFolderPath(source, true) + ".." +
		separator + "00_ground_truth" + separator + groundtruthFilename + ".png";
	Mat groundTruthImage;
	try
	{
		groundTruthImage = FileSystem::loadImage(groundTruthImagePath);
		cvtColor(groundTruthImage, groundTruthImage, CV_BGR2GRAY);
	}
	catch(...)
	{
		// In case we're scanning a folder where there exists no ground truth for.
		return -1;
	}

	int pixelcount;
	int equalpixels;

	cout << "Ground Truth File Name: " << groundtruthFilename << endl;
    //QR Code extraction failed
    if (groundTruthImage.size != outputImage.size)
    {
		cout << "Ground Truth Size: " << groundTruthImage.size() << endl;
		cout << "Output Size: " << outputImage.size() << endl;
		cout << "This Image has not the expected Size! No equality." << endl;
		return -1;
    }

    pixelcount = groundTruthImage.cols * groundTruthImage.rows;
    equalpixels = 0;
	uint8_t groundtruthPixelValue;
	uint8_t exatractedPixelValue;

    //iteration over all Pixel in the Image and check
    //the equality of the images
    for (int i = 0; i < groundTruthImage.cols; ++i) {
        for (int j = 0; j < groundTruthImage.rows; ++j) {
            groundtruthPixelValue = groundTruthImage.at<uint8_t>(i, j);
            exatractedPixelValue = outputImage.at<uint8_t>(i, j);
            if (groundtruthPixelValue == exatractedPixelValue)
                equalpixels++;
        }
    }

    equality = (float(equalpixels) / float(pixelcount)) * 100;

	cout << "Equality: " << equality << "% - Total Pixel: " << pixelcount << " - Equal Pixel: " << equalpixels << endl;
	return equality;
}
