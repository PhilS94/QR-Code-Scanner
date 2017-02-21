#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "FindPattern.hpp"

using namespace std;
using namespace cv;

/*
bool isLexicographicMax(Point a, Point b, Point c) {
	bool retA = false;
	bool retB = false;

	if (c.y > b.y || (c.y == b.y && c.x > b.x)) {
		retB = true;
	}

	if (c.y > a.y || (c.y == a.y && c.x > a.x)) {
		retA = true;
	}

	return retA && retB;

}
*/

int getOrientation(Point a, Point b, Point c) {
    return ((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x));
}

bool compareContourAreas(vector<Point> contour1, vector<Point> contour2) {
    double i = abs(contourArea(Mat(contour1)));
    double j = abs(contourArea(Mat(contour2)));

    return (i >= j && contour1[0].x < contour2[0].x);
}

bool compareContoureVectors(vector<vector<Point>> contours1, vector<vector<Point>> contours2) {
    double i = 0;
    double j = 0;

    for (int k = 0; k < contours1.size(); ++k) {
        i += abs(contourArea(Mat(contours1[k])));
    }

    for (int k = 0; k < contours1.size(); ++k) {
        j += abs(contourArea(Mat(contours2[k])));
    }

    return (i > j);
}


FindPattern::FindPattern(Mat originalImage) {
    this->originalImage = originalImage;
}

Mat FindPattern::findAllContours(Mat image) {

    float minPix = 8.00;
    float maxPix = 0.2 * image.cols * image.rows;

    image /= 255;
    cv::Mat contourOutput = image.clone();
    vector<Vec4i> hierarchy;
    findContours(image, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
    cout << "contours.size = " << contours.size() << endl;
    int m = 0;
    while (m < contours.size()) {
        if (contourArea(contours[m]) <= minPix) {
            contours.erase(contours.begin() + m);
        } else if (contourArea(contours[m]) > maxPix) {
            contours.erase(contours.begin() + m);
        } else ++m;
    }
    cout << "contours.size = " << contours.size() << endl;


    cv::Mat contourImage = originalImage.clone();
    cv::Scalar color[3];
    color[0] = cv::Scalar(0, 0, 255);
    color[1] = cv::Scalar(0, 255, 0);
    color[2] = cv::Scalar(255, 0, 0);

    for (size_t idx = 0; idx < contours.size(); idx++) {
        cv::drawContours(contourImage, contours, idx, color[idx % 3]);
    }
    return contourImage;
}

Mat FindPattern::findQRCodePatterns(Mat image) {
	// TODO: What happens if we find more than three patterns?
    sort(contours.begin(), contours.end(), compareContourAreas);

    for (int i = 0; i < contours.size(); ++i) {
        //iteration über die Kontur Punkte
        for (int j = 0; j < contours.size(); ++j) {
            if (isContourInsideContour(contours.at(i), contours.at(j))) {
                for (int k = 0; k < contours.size(); ++k) {
                    if (isContourInsideContour(contours.at(k), contours.at(i))) {
                        if (isTrapez(contours.at(j))) {
                            vector<vector<Point>> pairs;
                            pairs.push_back((vector<Point> &&) contours.at(j));
                            pairs.push_back((vector<Point> &&) contours.at(i));
                            pairs.push_back((vector<Point> &&) contours.at(k));
                            trueContoures.push_back(pairs);
                        }
                    }
                }
            }
        }
    }

    sort(trueContoures.begin(), trueContoures.end(), compareContoureVectors);

    cv::Mat contourImage = originalImage.clone();

    cv::Scalar color[3];
    color[0] = cv::Scalar(0, 0, 255);
    color[1] = cv::Scalar(0, 255, 0);
    color[2] = cv::Scalar(255, 0, 0);
    for (size_t idx = 0; idx < trueContoures.size(); idx++) {
        for (size_t idx2 = 0; idx2 < trueContoures.at(idx).size(); idx2++) {
            cv::drawContours(contourImage, trueContoures[idx], idx2, color[idx % 3]);
        }
    }
    return contourImage;

}

bool FindPattern::isContourInsideContour(std::vector<cv::Point> in, std::vector<cv::Point> out) {
    if (in.size() > 0 && out.size() > 0) {
        for (int i = 0; i < in.size(); i++) {
            if (pointPolygonTest(out, in[i], false) <= 0) return false;
        }
        return true;
    }

    return false;
}

bool FindPattern::isTrapez(std::vector<cv::Point> in) {

    std::vector<cv::Point> approximatedPolygon;
    double epsilon = 0.1 * cv::arcLength(in, true);
    approxPolyDP(in, approximatedPolygon, epsilon, true);
    bool ret = (approximatedPolygon.size() == 4);
    return ret;
}

Point FindPattern::calculateMassCentres(std::vector<cv::Point> in) {

    int xCoordinates[] = {std::numeric_limits<int>::max(), std::numeric_limits<int>::min()};
    int yCoordinates[] = {std::numeric_limits<int>::max(), std::numeric_limits<int>::min()};

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

Mat FindPattern::tiltCorrection(Mat image, FinderPatternModel fPattern) {
    vector<Point2f> vecsrc;
    vector<Point2f> vecdst;

    vecdst.push_back(Point2f(20, 20));
    vecdst.push_back(Point2f(120, 20));
    vecdst.push_back(Point2f(20, 120));


    vecsrc.push_back(fPattern.topleft);
    vecsrc.push_back(fPattern.topright);
    vecsrc.push_back(fPattern.bottomleft);


    Mat affineTrans = getAffineTransform(vecsrc, vecdst);
    Mat warped = Mat(image.size(), image.type());
    warpAffine(image, warped, affineTrans, image.size());

    // TODO: Improve warp target roi size!
    int width = image.size().width < 145 ? image.size().width : 145;
    int height = image.size().height < 145 ? image.size().height : 145;

    Mat qrcode_color = warped(Rect(0, 0, width, height));
    Mat qrcode_gray;
    cvtColor(qrcode_color, qrcode_gray, CV_BGR2GRAY);
    Mat qrcode_bin;
    threshold(qrcode_gray, qrcode_bin, 120, 255, CV_THRESH_OTSU);

    return qrcode_bin;
}

Mat FindPattern::normalize(Mat image) {
    if (image.cols != image.rows) {
        //Can't normalize a non-quadratic Image
        throw std::exception();
    }

    int versionNumber = getVersionNumber(image);
    int modules = getModules(versionNumber);

    Mat scaledImage;
    resize(image, scaledImage, Size(), 2, 2, CV_INTER_LINEAR);

    // Create modules x modules Grid over scaledImage and use the Pixel at the center of the each cell as the Pixel for the trueSizedQRImage
    Mat trueSizeQRImage = Mat(modules, modules, CV_8U);
    float gridLength = scaledImage.cols / modules;
    Point startPoint = Point(gridLength / 2, gridLength / 2);

    for (int x = 0; x < modules; x++) {
        for (int y = 0; y < modules; y++) {
            Point centerOfCell = startPoint + Point(x * gridLength, y * gridLength);
            centerOfCell = Point(cvRound(centerOfCell.x), cvRound(centerOfCell.y));
            trueSizeQRImage.at<uchar>(x, y) = scaledImage.at<uchar>(centerOfCell);
        }
    }

    return trueSizeQRImage;
}

int FindPattern::getVersionNumber(Mat image) {
    //TODO: Lese Versionsnummer des QRCodes ab
    return 1;
}

int FindPattern::getModules(int versionNumber) {
    if ((versionNumber < 1) || (versionNumber > 40)) {
        //Versionnumber of QRCode is not legit.
        throw std::exception();
    }
    return 21 + (versionNumber - 1) * 4;
    /*This return the following:
    1:	21
    2:	25
    3:	29
    4:	33
    ...
    */
}

FinderPatternModel FindPattern::getFinderPatternModel(vector<Point> cont1, vector<Point> cont2, vector<Point> cont3) {
    Point pt1 = calculateMassCentres(cont1);
    Point pt2 = calculateMassCentres(cont2);
    Point pt3 = calculateMassCentres(cont3);

    double d12 = sqrt(pow(abs(pt1.x - pt2.x), 2) + pow(abs(pt1.y - pt2.y), 2));
    double d13 = sqrt(pow(abs(pt1.x - pt3.x), 2) + pow(abs(pt1.y - pt3.y), 2));
    double d23 = sqrt(pow(abs(pt2.x - pt3.x), 2) + pow(abs(pt2.y - pt3.y), 2));

    double Max = max(d12, max(d13, d23));
    Point a, b, c; //a linksoben

    if (Max == d12) {
        a = pt3;
        b = pt1;
        c = pt2;
    } else if (Max == d13) {
        a = pt2;
        b = pt1;
        c = pt3;
    } else if (Max == d23) {
        a = pt1;
        b = pt2;
        c = pt3;
    }

    //Überprüfe die Orientierung des Dreiecks a,b,c und entscheide ob das Dreieck a,b,c oder a,c,b das gewünnschte Dreieck ist.
    if (getOrientation(a, b, c) > 0) {
        FinderPatternModel patternModel(a, b, c);
        return patternModel;
    }

	if (getOrientation(a, b, c) < 0) {
		FinderPatternModel patternModel(a, c, b);
		return patternModel;
	}

	//The three detected Points lie on a line! No legit QR-Code!
	throw exception();
}


/*
FinderPatternModel FindPattern::getFinderPatternModel(vector<Point> cont1, vector<Point> cont2, vector<Point> cont3) {


	Point pt1 = calculateMassCentres(cont1);
	Point pt2 = calculateMassCentres(cont2);
	Point pt3 = calculateMassCentres(cont3);

	double d12 = sqrt(pow(abs(pt1.x - pt2.x), 2) + pow(abs(pt1.y - pt2.y), 2));
	double d13 = sqrt(pow(abs(pt1.x - pt3.x), 2) + pow(abs(pt1.y - pt3.y), 2));
	double d23 = sqrt(pow(abs(pt2.x - pt3.x), 2) + pow(abs(pt2.y - pt3.y), 2));

	double x1, y1, x2, y2, x3, y3;
	double Max = max(d12, max(d13, d23));
	Point p1, p2, p3;
	if (Max == d12) {
		p1 = pt1;
		p2 = pt2;
		p3 = pt3;
	}
	else if (Max == d13) {
		p1 = pt1;
		p2 = pt3;
		p3 = pt2;
	}
	else if (Max == d23) {
		p1 = pt2;
		p2 = pt3;
		p3 = pt1;
	}

	if (getOrientation(p1, p2, p3) < 0) {
		if (isLexicographicMax(p1, p2, p3)) {
			if (p1.x < p2.x) {
				FinderPatternModel patternModel(p3, p2, p1);
				return patternModel;
			}
			else {
				FinderPatternModel patternModel(p3, p1, p2);
				return patternModel;
			}
		}
		else {
			if (p1.x < p2.x) {
				FinderPatternModel patternModel(p3, p1, p2);
				return patternModel;
			}
			else {
				FinderPatternModel patternModel(p3, p1, p2);
				return patternModel;
			}
		}
	}
	else {
		if (isLexicographicMax(p1, p3, p2)) {
			if (p1.y < p3.y) {
				FinderPatternModel patternModel(p1, p3, p2);
				return patternModel;
			}
			else {
				FinderPatternModel patternModel(p3, p1, p2);
				return patternModel;
			}
		}
		else {
			if (p2.y < p3.y) {
				FinderPatternModel patternModel(p3, p1, p2);
				return patternModel;
			}
			else {
				FinderPatternModel patternModel(p3, p2, p1);
				return patternModel;
			}
		}
	}
}
*/

void FindPattern::getAllPatterns(vector<FinderPatternModel> &patterns) {

    std::cout << "trueContoures Size: " << trueContoures.size() << endl;

    //Philipp: Diese Funktion hat Fehlermeldung wenn sie mit trueContoures.size()=2 durchlaufen wird! Und das passiert bei einigen Bildern.
    //Vorzeitige Lösung:
    if (trueContoures.size() >= 3) {
        for (int i = 0; i < trueContoures.size(); i++) {
            patterns.push_back(getFinderPatternModel(trueContoures[i][trueContoures[i].size() - 1],
                                                     trueContoures[i++][trueContoures[i].size() - 1],
                                                     trueContoures[i++][trueContoures[i].size() - 1]));
        }
    }
}


