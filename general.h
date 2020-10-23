#pragma once

// A set of general operators

#include"Header.h"

class Customer_Solomon {
public:
	int id;
	double xCoord;
	double yCoord;
	double demand;
	double readyTime;
	double dueTime;
	double serviceTime;
};

class Instance_Solomon {
public:
	string name;
	int numVehicle;
	double capacity;
	vector<Customer_Solomon> vertices;
};

// Euclidean distance.
double EuclideanDistance(const double x1, const double y1, const double x2, const double y2);
// Set precision.
double setPrecision(const double data, const int numFloat);


