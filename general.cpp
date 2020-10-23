#include"general.h"

// Euclidean distance.
double EuclideanDistance(const double x1, const double y1, const double x2, const double y2) {
	return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}


// Set precision.
// NB. Ceil function is used.
double setPrecision(const double data, const int numFloat) {
	return ceil(data * pow(10, numFloat)) / pow(10, numFloat);
}


