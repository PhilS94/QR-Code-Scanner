#include <iostream>
#include <cv.h>
#include <opencv2/imgproc/imgproc.hpp>
#include "ImageReader.hpp"
#include "ImageBinarization.hpp"
#include "FindPatern.hpp"
#include "FinderPaternModel.hpp"



using namespace std;
using namespace cv;

void videoInput();

void pictureInput(const string path);

int main(int argc, const char *argv[]) {
    cout << argv[0] << endl;

    if (argc > 3 || argc == 1) {
        cout << "Unknown option! \nusage: <main> [0 = picture, 1 = camera ] [<path>]" << endl;
    } else {
        if (argc == 2) {
            cout << argv[0] << endl;
            videoInput();
        } else if (argc == 3) {
            cout << argv[2] << endl;
            string s;
            s += argv[2];
            pictureInput(s);
        } else {
            return -1;
        }
    }

    waitKey(0);
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
        namedWindow("Konturen #2", WINDOW_AUTOSIZE);
        imshow("Konturen #2", filteredContours);

        // Press 'c' to escape
        if (waitKey(30) == 'c') break;
    }
}

void pictureInput(const string path) {

    ImageReader reader;
    Mat image = reader.readImage(path);
    if (image.cols > 2000 || image.rows > 2000) {
        resize(image, image, image.size(), 0.25, 0.25, INTER_LINEAR);
    }


    Mat grayscale;
    cvtColor(image, grayscale, CV_BGR2GRAY);
    namedWindow( "Grayscale", WINDOW_AUTOSIZE );
    imshow("Grayscale", grayscale);

    Mat binary;
    ImageBinarization binarizedImage;
    binary = binarizedImage.run(grayscale);
    namedWindow( "Binaer", WINDOW_AUTOSIZE );
    imshow("Binaer", binary);


    Mat contour;
    FindPatern patern(image);
    contour = patern.findAllContours(binary);
    namedWindow( "Konturen", WINDOW_AUTOSIZE );
    imshow("Konturen", contour);

    Mat filteredContours;
    filteredContours = patern.findQRCodePaterns(binary);
    namedWindow("Konturen #2", WINDOW_AUTOSIZE);
    imshow("Konturen #2", filteredContours);

    vector<FinderPaternModel> fPattern = patern.getAllPaterns();

    cv::Scalar color[3];
    color[0] = cv::Scalar(0, 0, 255);
    color[1] = cv::Scalar(0, 255, 0);
    color[2] = cv::Scalar(255, 0, 0);

    for (int i = 0; i < fPattern.size(); ++i) {
        cout << "############" <<endl;
        cout << fPattern[i].topleft.x << ", "<< fPattern[i].topleft.y <<endl;
        cout << fPattern[i].topright.x << ", "<< fPattern[i].topright.y <<endl;
        cout << fPattern[i].bottomleft.x << ", "<< fPattern[i].bottomleft.y <<endl;
        circle(image, fPattern[i].topleft, 3, color[i % 3], 2, 8, 0);
        circle(image, fPattern[i].topright, 3, color[i % 3], 2, 8, 0);
        circle(image, fPattern[i].bottomleft, 3, color[i % 3], 2, 8, 0);
        cout << "############" <<endl;
    }

    namedWindow("Neues Image", WINDOW_AUTOSIZE);
    imshow("Neues Image", image);

    Mat images1, images2, images3;
    images1 = patern.tiltCorrection(image, fPattern[0]);
    images2 = patern.tiltCorrection(image, fPattern[1]);
    images3 = patern.tiltCorrection(image, fPattern[2]);

    namedWindow("QR Code1", WINDOW_AUTOSIZE);
    imshow("QR Code1", images1);

    namedWindow("QR Code2", WINDOW_AUTOSIZE);
    imshow("QR Code2", images2);

    namedWindow("QR Code3", WINDOW_AUTOSIZE);
    imshow("QR Code3", images3);
}