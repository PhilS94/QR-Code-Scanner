#include <iostream>
#include <string>
#include <opencv2/opencv.hpp> // Philipp: Besser als das include #include <cv.h> (verursachte Fehlermeldungen)
#include <opencv2/imgproc/imgproc.hpp>
#include <sys/stat.h>
#include <sys/types.h>
#include "ImageReader.hpp"
#include "ImageBinarization.hpp"
#include "FindPatern.hpp"
#include "FinderPaternModel.hpp"

#ifdef _WIN32
#include <direct.h>
#endif


using namespace std;
using namespace cv;

void videoInput();

void pictureInput(const string path);


inline char separator() {
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
};

inline void makeDir(string strPath) {

    struct stat info;
    const char *tmp = strPath.c_str();

    if (stat(tmp, &info) != 0) {
#ifdef _WIN32
		_mkdir(strPath.c_str());
#else
        mkdir(strPath.c_str(), 0777);
#endif
    }
}

/**
Philipp: Lädt alle Bilder am angegebenen Pfad und wendet QR-Code Algorithmus auf sie an.

@param Pfad an dem Dateien gesucht werden.
*/
void testAllImagesAtPath(const string path);

int main(int argc, const char *argv[]) {
    cout << argv[0] << endl;

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

    if (argc > 3 || argc == 1) {
        cout << "|                                                                           |" << endl;
        cout << "| Please enter 0 and a valid Path to load a Directory with Pictures.        |\n"
                "| e.g. 0 /home/username/Pictures                                            |\n"
                "| For starting the Program with Videoinput please enter 1                   |\n"
                "|                                                                           |" << endl;
        cout << "| Usage: <main> [0 = picture, 1 = camera ] [<path>]                         |\n"
                "+---------------------------------------------------------------------------+" << endl;
    } else {
        if (argc == 2) {
            cout << argv[0] << endl;
            videoInput();
        } else if (argc == 3) {
            cout << argv[2] << endl;
            string s;
            s += argv[2];
            testAllImagesAtPath(s); // Philipp: Statt pictureInput diese Funktion eingefügt
        } else {
            return -1;
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
        //    namedWindow( "Grayscale", WINDOW_AUTOSIZE );
        //    imshow("Grayscale", grayscale);

        Mat binary;
        ImageBinarization binarizedImage;
        binary = binarizedImage.run(grayscale);
        //    namedWindow( "Binaer", WINDOW_AUTOSIZE );
        //    imshow("Binaer", binary);


        Mat contour;
        FindPatern patern(frame);
        contour = patern.findAllContours(binary);
        //    namedWindow( "Konturen", WINDOW_AUTOSIZE );
        //    imshow("Konturen", contour);

        Mat filteredContours;
        filteredContours = patern.findQRCodePaterns(binary);

        //DEBUG
        cout << "Now getting all Patterns" << endl;
        vector<FinderPaternModel> fPattern;
        patern.getAllPaterns(fPattern); //Philipp: Liefert nicht immer Vector mit 3Elementen, sondern mit 0 oder 1 Elementen.

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
            frame = patern.tiltCorrection(frame, fPattern[0]);
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
    std::string imageName = path.substr(0, path.find_last_of(".")).substr(path.find_last_of(separator()) + 1);
    std::string currentDir = path.substr(0, path.find_last_of(separator()) + 1);
    cout << endl;

    cout << "Current Directory: " << currentDir << endl;
    cout << "Now Loading Image: " << imageName << endl;

    ImageReader reader;
    Mat image = reader.readImage(path);
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
    //namedWindow(imageName+" Grayscale", WINDOW_AUTOSIZE );
    //imshow(imageName+" Grayscale", grayscale);

    cout << "Now binarizing Image..." << endl;
    Mat binary;
    ImageBinarization binarizedImage;
    binary = binarizedImage.run(grayscale);
    //namedWindow(imageName+" Binaer", WINDOW_AUTOSIZE );
    //imshow(imageName+" Binaer", binary);

    cout << "Now finding Contours..." << endl;
    Mat contour;
    FindPatern patern(image);
    contour = patern.findAllContours(binary);
    //namedWindow(imageName+" Konturen", WINDOW_AUTOSIZE );
    //imshow(imageName+" Konturen", contour);

    cout << "Now filtering Contours..." << endl;
    Mat filteredContours;
    filteredContours = patern.findQRCodePaterns(binary);
    //namedWindow(imageName+" Konturen #2", WINDOW_AUTOSIZE);
    //imshow(imageName+" Konturen #2", filteredContours);

    cout << "Now getting all Patterns" << endl;
    vector<FinderPaternModel> fPattern;
    patern.getAllPaterns(fPattern); //Philipp: Liefert nicht immer Vector mit 3Elementen, sondern mit 0 oder 1 Elementen.

    cv::Scalar color[3];
    color[0] = cv::Scalar(0, 0, 255);
    color[1] = cv::Scalar(0, 255, 0);
    color[2] = cv::Scalar(255, 0, 0);

    for (int i = 0; i < fPattern.size(); ++i) {
//		cout << "############" << endl;
//		cout << fPattern[i].topleft.x << ", " << fPattern[i].topleft.y << endl;
//		cout << fPattern[i].topright.x << ", " << fPattern[i].topright.y << endl;
//		cout << fPattern[i].bottomleft.x << ", " << fPattern[i].bottomleft.y << endl;
        circle(image, fPattern[i].topleft, 3, color[i % 3], 2, 8, 0);
        circle(image, fPattern[i].topright, 3, color[i % 3], 2, 8, 0);
        circle(image, fPattern[i].bottomleft, 3, color[i % 3], 2, 8, 0);
//		cout << "############" << endl;
    }

    //namedWindow(imageName+" Tracked Image", WINDOW_AUTOSIZE);
    //imshow(imageName+" Tracked Image", image);

    cout << "Now tiltCorrecting Image...: " << endl;

    //Philipp: fPattern enthält nicht immer 3 Elemente->Vector out of bounds error, tatsächlich immer nur 0 oder 1 Element. vorzeitige Lösung:
    Mat image1, image2, image3;
    if (fPattern.size() > 0) {
        image1 = patern.tiltCorrection(image, fPattern[0]);
        //namedWindow(imageName + " QR Code1", WINDOW_AUTOSIZE);
        //imshow(imageName + " QR Code1", image1);
    }
    if (fPattern.size() > 1) {
        image2 = patern.tiltCorrection(image, fPattern[1]);
        namedWindow(imageName + " QR Code2", WINDOW_AUTOSIZE);
        imshow(imageName + " QR Code2", image2);
    }
    if (fPattern.size() > 2) {
        image3 = patern.tiltCorrection(image, fPattern[2]);
        namedWindow(imageName + " QR Code3", WINDOW_AUTOSIZE);
        imshow(imageName + " QR Code3", image3);
    }

    /*Philipp:
    Damit die Bilder gespeichert werden, muss ein Ordner "ScannedQR" im ProjektOrdner zunächst manuell erstellt werden.
    Alternativ setze saveDir auf leeren String "", dann werden alle Bilder direkt im ProjektOrdner gespeichert.
    */


    string saveDir = currentDir + "ScannedQR/";
    makeDir(saveDir);

    string saveImageName = imageName + "_QRScanned.jpg";
    cout << "Saving QR-Tracked Image in " << saveDir + saveImageName << endl;
    imwrite(saveDir + saveImageName, image);

    if (!image1.empty()) {
        string saveImageExtractedName = imageName + "_QRExtracted.jpg";
        cout << "Saving QR-Extracted Image in " << saveDir + saveImageExtractedName << endl;
        imwrite(saveDir + saveImageExtractedName, image1); // Speichere momentan nur image1..
    }
}

void testAllImagesAtPath(const string path) {
    cout << "Reading all Files in " << path << " ..." << endl;

    vector<cv::String> allFiles;
    vector<std::string> validFiles;
    cv::glob(path, allFiles, false); // Liest alle Dateinamen am gegebenen Pfad in allFiles ein

    //Filter alle Dateien außer .jpg und .png aus, ggf. noch andere zulassen?
    for (vector<String>::iterator it = allFiles.begin(); it != allFiles.end(); ++it) {
        std::string file = String(*it);
        std::string fileType = file.substr(file.find_last_of(".") + 1);
        if ((fileType == "jpg") || (fileType == "png")) {
            validFiles.push_back(file);
        }
    }

    cout << "Found the following valid Files: " << endl;
    for (vector<std::string>::iterator it = validFiles.begin(); it != validFiles.end(); ++it) {
        cout << *it << endl;
    }
    cout << endl;

    char confirm;
    cout << "Do you want to read all these files and test QR-Code Algorithm? If yes press 'y':  ";
    cin >> confirm;

    if (confirm == 'y') {
        cout << "Now start iterating through all Images.." << endl;
        for (vector<std::string>::iterator it = validFiles.begin(); it != validFiles.end(); ++it) {
            pictureInput(*it);
        }
        cout << endl;
        cout << "Finished iterating through all Images." << endl;
    }
}