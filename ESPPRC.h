#pragma once

#include"Header.h"
#include"define.h"

constexpr auto Max_Num_Vertex = 128;

class Data_ESPPRC {
public:
	vector<vector<bool>> ExistingArcs;
	vector<vector<QuantityType>> Quantity;
	vector<pair<QuantityType, QuantityType>> QuantityWindow;
	vector<vector<DistanceType>> Distance;
	vector<pair<DistanceType, DistanceType>> DistanceWindow;
	vector<vector<TimeType>> Time;
	vector<pair<TimeType, TimeType>> TimeWindow;
	vector<vector<double>> RealCost;
	vector<vector<double>> ReducedCost;
};

class Consumption_ESPPRC {
public:
	QuantityType quantity;
	DistanceType distance;
	TimeType time;
};

class Cost_ESPPRC {
public:
	double realCost;
	double reducedCost;
};

class Label_ESPPRC {
public:
	vector<int> path;
	int tail;									// the last vertex in the path
	bitset<Max_Num_Vertex> unreachable;
	Consumption_ESPPRC consumption;
	Cost_ESPPRC cost;

	bool canExtend(const int j) const;
};

