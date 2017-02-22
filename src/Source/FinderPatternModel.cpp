#include "../Header/FinderPatternModel.hpp"

using namespace std;
using namespace cv;

FinderPatternModel::FinderPatternModel(Point a, Point b, Point c) {
    this->topleft = a;
    this->topright = b;
    this->bottomleft = c;
}


