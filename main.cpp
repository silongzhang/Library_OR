#include"Header.h"
#include"general.h"
#include"ESPPRC.h"

extern clock_t start;
extern clock_t last;

int main(int argc, char** argv) {
	start = clock();
	last = start;

	string folderSolomon = "data//solomon instances//solomon_100//";
	string fileSolomon = "C101.txt";
	string fileInputSolomon = folderSolomon + fileSolomon;
	Instance_Solomon inst = readSolomonInstance(fileInputSolomon);

	double coefDist = 0.2;
	double prz = 25;
	vector<double> prize(inst.vertices.size(), prz);
	prize[0] = 0;
	int precision = 1;

	// Parameters.
	Data_Input_ESPPRC data;
	data.sizeQuantLB = 30;
	data.sizeDistLB = 30;
	data.sizeTimeLB = 30;
	data.maxReducedCost = 0;
	data.maxNumRoutesReturned = 10;
	data.allowPrintLog = true;
	data.applyLB = { true,true,true };

	// File operator.
	string instanceName(fileSolomon.begin(), fileSolomon.end() - string(".txt").length());

	string folderInputESPPRC = "data//ESPPRC//input//";
	string fileInputESPPRC = folderInputESPPRC + instanceName + ".txt";

	string folderOutputESPPRC = "data//ESPPRC//output//";
	string fileOutputESPPRC = folderOutputESPPRC + instanceName + ".txt";
	
	readDataSolomonESPPRC(inst, data, coefDist, prize, precision);
	
	writeToFile(data, fileInputESPPRC);

	readFromFile(data, fileInputESPPRC);

	data.incrementQuantLB = floor((data.QuantityWindow[0].second - data.QuantityWindow[0].first) / data.sizeQuantLB);
	data.incrementDistLB = floor((data.DistanceWindow[0].second - data.DistanceWindow[0].first) / data.sizeDistLB);
	data.incrementTimeLB = floor((data.TimeWindow[0].second - data.TimeWindow[0].first) / data.sizeTimeLB);

	// Algorithm.
	data.preprocess();
	Data_Auxiliary_ESPPRC auxiliary;
	auto result = DPAlgorithmESPPRC(data, auxiliary, cout);
	printResultsDPAlgorithmESPPRC(data, auxiliary, cout, result);

	return 0;
}

