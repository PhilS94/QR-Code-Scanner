#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <sys/stat.h>
#include "ImageBinarization.hpp"
#include "FindPattern.hpp"
#include "Filesystem.hpp"
#include "Generate.hpp"

using namespace std;
using namespace cv;

void videoInput();
void pictureInput(const string path);
void testAllImagesAtPath(const string path);
void generateMode(const string source, const string dest);

void printLogo()
{
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

void printUsage()
{
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

	if (argc == 1)
	{
		cout << "Starting Camera Mode..." << endl;
		videoInput();
	}
	else if (argc == 2)
	{
		cout << "Starting Folder Scan Mode..." << endl;
		cout << argv[1] << endl;
		string s;
		s += argv[1];
		testAllImagesAtPath(s);
	}
	else if (argc == 3)
	{
		cout << "Starting Evaluation Mode..." << endl;
		// TODO: Start Evaluation mode
		// In this mode the first argument is a path value for the input and the second argument
		// is a path value for the resulting output file for matching with the ground truth.
	}
	else if (argc == 4 && string(argv[1]) == "-generate")
	{
		cout << "Starting Generate Mode..." << endl;
		generateMode(argv[2], argv[3]);
	}
	else
	{
		printUsage();
		cout << endl << endl << "Printing Arguments:" << endl << endl;
		for (int i = 0; i < argc; i++)
		{
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

void videoInput() {
	VideoCapture cap(0); // open the default camera
	if (!cap.isOpened())  // check if we succeeded
		cout << "Errorcode: -1" << endl;

	namedWindow("Video", 1);
	while (1) {
		Mat frame;
		cap >> frame;         // get a new frame from camera
		imshow("Video", frame);


		Mat grayscale;
		cv::cvtColor(frame, grayscale, CV_BGR2GRAY);
		namedWindow("Grayscale", WINDOW_AUTOSIZE);
		imshow("Grayscale", grayscale);

		Mat binary;
		ImageBinarization binarizedImage;
		binary = binarizedImage.run(grayscale);
		namedWindow("Binaer", WINDOW_AUTOSIZE);
		imshow("Binaer", binary);


		Mat contour;
		FindPattern pattern(frame);
		contour = pattern.findAllContours(binary);
		namedWindow("Konturen", WINDOW_AUTOSIZE);
		imshow("Konturen", contour);

		Mat filteredContours;
		filteredContours = pattern.findQRCodePatterns(binary);

		//DEBUG
		cout << "Now getting all Patterns" << endl;
		vector<FinderPatternModel> fPattern;
		pattern.getAllPatterns(
			fPattern); //Philipp: Liefert nicht immer Vector mit 3Elementen, sondern mit 0 oder 1 Elementen.

		cv::Scalar color[3];
		color[0] = cv::Scalar(0, 0, 255);
		color[1] = cv::Scalar(0, 255, 0);
		color[2] = cv::Scalar(255, 0, 0);

		for (int i = 0; i < fPattern.size(); ++i) {
			circle(frame, fPattern[i].topleft, 3, color[0], 2, 8, 0);
			circle(frame, fPattern[i].topright, 3, color[1], 2, 8, 0);
			circle(frame, fPattern[i].bottomleft, 3, color[2], 2, 8, 0);
		}

		//namedWindow(imageName+" Tracked Image", WINDOW_AUTOSIZE);
		//imshow(imageName+" Tracked Image", image);

		cout << "Now tiltCorrecting Image...: " << endl;

		//Philipp: fPattern enthält nicht immer 3 Elemente->Vector out of bounds error, tatsächlich immer nur 0 oder 1 Element. vorzeitige Lösung:
		if (fPattern.size() > 0) {
			frame = pattern.tiltCorrection(frame, fPattern[0]);
		}

		string saveImageName = "QRScanned";

		if (!frame.empty()) {
			imshow(saveImageName, frame); // Speichere momentan nur image1..
		}
		//DEBUG

		// Press 'c' to escape
		if (waitKey(30) == 'c') break;
	}
}


void pictureInput(const string path) {

	//Philipp: extrahiert den Namen der Datei. Nützlich für Bildvorschau und dem folgenden speichern der Datei. Ist aber relativ unsichere Lösung.
	std::string imageName = FileSystem::toFileName(path);
	std::string currentDir = FileSystem::toFolderPath(path, true);
	cout << endl;

	cout << "Now Loading Image: " << imageName << endl;

	Mat image = FileSystem::readImage(path);
	if (image.cols > 2000 || image.rows > 2000) {
		cout << "Now resizing Image, because it is too large: " << image.rows << "x" << image.cols << ". ";
		Mat resizedImage(0.25 * image.rows, 0.25 * image.cols, image.type());
		resize(image, resizedImage, resizedImage.size(), 0.25, 0.25, INTER_LINEAR);
		image = resizedImage;
		cout << "New size: " << image.rows << "x" << image.cols << "." << endl;
	}

	cout << "Now Converting Image to Grayscale..." << endl;
	Mat grayscale;
	cvtColor(image, grayscale, CV_BGR2GRAY);
	//    namedWindow(imageName+" Grayscale", WINDOW_AUTOSIZE );
	//    imshow(imageName+" Grayscale", grayscale);

	cout << "Now binarizing Image..." << endl;
	Mat binary;
	ImageBinarization binarizedImage;
	binary = binarizedImage.run(grayscale);
	//    namedWindow(imageName+" Binaer", WINDOW_AUTOSIZE );
	//    imshow(imageName+" Binaer", binary);

	cout << "Now finding Contours..." << endl;
	Mat contour;
	FindPattern pattern(image);
	contour = pattern.findAllContours(binary);


	cout << "Now filtering Contours..." << endl;
	Mat filteredContours;
	filteredContours = pattern.findQRCodePatterns(binary);
	//    namedWindow(imageName+" Konturen #2", WINDOW_AUTOSIZE);
	//    imshow(imageName+" Konturen #2", filteredContours);

	cout << "Now getting all Patterns" << endl;
	vector<FinderPatternModel> fPattern;
	pattern.getAllPatterns(fPattern); //Philipp: Liefert nicht immer Vector mit 3Elementen, sondern mit 0 oder 1 Elementen.

	cv::Scalar color[3];
	color[0] = cv::Scalar(0, 0, 255);
	color[1] = cv::Scalar(0, 255, 0);
	color[2] = cv::Scalar(255, 0, 0);

	for (int i = 0; i < fPattern.size(); ++i) {
		circle(image, fPattern[i].topleft, 3, color[i % 3], 2, 8, 0);
		circle(image, fPattern[i].topright, 3, color[i % 3], 2, 8, 0);
		circle(image, fPattern[i].bottomleft, 3, color[i % 3], 2, 8, 0);
	}

	//namedWindow(imageName+" Tracked Image", WINDOW_AUTOSIZE);
	//imshow(imageName+" Tracked Image", image);

	cout << "Now tiltCorrecting Image...: " << endl;

	//Philipp: fPattern enthält nicht immer 3 Elemente->Vector out of bounds error, tatsächlich immer nur 0 oder 1 Element. vorzeitige Lösung:
	Mat QRCode;
	Mat QRCodeTrueSize;
	if (fPattern.size() > 0) {
		QRCode = pattern.tiltCorrection(image, fPattern[0]);
		QRCodeTrueSize = pattern.normalize(QRCode);
		//namedWindow(imageName + " QR Code1", WINDOW_AUTOSIZE);
		//imshow(imageName + " QR Code1", QRCode);
	}

	/*Philipp:
	Damit die Bilder gespeichert werden, muss ein Ordner "ScannedQR" im ProjektOrdner zunächst manuell erstellt werden.
	Alternativ setze saveDir auf leeren String "", dann werden alle Bilder direkt im ProjektOrdner gespeichert.
	*/

	string saveDir = currentDir + "ScannedQR/";
	FileSystem::makeDir(saveDir);

	string saveImageName = imageName + "_1_QRScanned.jpg";
	cout << "Saving QR-Tracked Image in " << saveDir + saveImageName << endl;
	imwrite(saveDir + saveImageName, image);

	if (!QRCode.empty()) {
		string saveImageExtractedName = imageName + "_2_QRExtracted.jpg";
		cout << "Saving QR-Extracted Image in " << saveDir + saveImageExtractedName << endl;
		imwrite(saveDir + saveImageExtractedName, QRCode); 
	}

	if (!QRCodeTrueSize.empty()) {
		string saveQRTrueSizeName = imageName + "_3_QRTrueSize.jpg";
		cout << "Saving QR-Extracted Image in " << saveDir + saveQRTrueSizeName << endl;
		imwrite(saveDir + saveQRTrueSizeName, QRCodeTrueSize);
	}
}

void testAllImagesAtPath(const string path) {
	cout << "Reading all Files in " << path << " ..." << endl;

	vector<std::string> imageFiles = FileSystem::allImagesAtPath(path);

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
		for (auto it = imageFiles.begin(); it != imageFiles.end(); ++it) {
			pictureInput(*it);
		}
		cout << endl;
		cout << "Finished iterating through all Images." << endl;
	}
}

void generateMode(const string source, const string dest)
{
	cout << "Source     : " << source << endl;
	cout << "Destination: " << dest << endl;

	Generator gen(source, dest);
	gen.border();
	gen.scale();
	gen.rotate();
}