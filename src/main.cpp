#include <iostream>
#include <cv.h>
#include <opencv2/imgproc/imgproc.hpp>
#include "ImageReader.hpp"
#include "ImageBinarization.hpp"
#include "FindPatern.hpp"



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
            //videoInput();
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
//    namedWindow( "Grayscale", WINDOW_AUTOSIZE );
//    imshow("Grayscale", grayscale);

    Mat binary;
    ImageBinarization binarizedImage;
    binary = binarizedImage.run(grayscale);
//    namedWindow( "Binaer", WINDOW_AUTOSIZE );
//    imshow("Binaer", binary);


    Mat contour;
    FindPatern patern(image);
    contour = patern.findAllContours(binary);
//    namedWindow( "Konturen", WINDOW_AUTOSIZE );
//    imshow("Konturen", contour);

    Mat filteredContours;
    filteredContours = patern.findQRCodePaterns(binary);
    namedWindow("Konturen #2", WINDOW_AUTOSIZE);
    imshow("Konturen #2", filteredContours);

}