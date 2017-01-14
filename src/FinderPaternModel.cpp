#include <opencv2/ml.hpp> // Philipp: Besser als das include #include <ml.h> (verursachte Fehlermeldungen) Wofür überhaupt MachineLearning?
#include "FinderPaternModel.hpp"

FinderPaternModel::FinderPaternModel(Point a, Point b, Point c){
    this->topleft    = a;
    this->topright   = b;
    this->bottomleft = c;
}


