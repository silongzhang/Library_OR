#include"Header.h"
#include"general.h"
#include"ESPPRC.h"

extern clock_t start;
extern clock_t last;

int main(int argc, char** argv) {
	start = clock();
	last = start;

	string folderSolomon = "data//solomon instances//solomon-100//";
	string fileSolomon = "c101.txt";
	string fileInputSolomon = folderSolomon + fileSolomon;
	Instance_Solomon inst = readSolomonInstance(fileInputSolomon);

	double coefDist = 0.2;
	double prz = 30;
	vector<double> prize(inst.vertices.size(), prz);
	prize[0] = 0;
	int precision = 1;

	// Parameters.
	Data_Input_ESPPRC data;
	data.incrementQuantLB = 20;
	data.sizeQuantLB = 7;
	data.incrementDistLB = 30;
	data.sizeDistLB = 6;
	data.incrementTimeLB = 200;
	data.sizeTimeLB = 5;
	data.maxReducedCost = 0;
	data.maxNumRoutesReturned = 100;
	data.allowPrintLog = true;
	vector<bool> applyLB = { true,true,true };

	// File operator.
	string instanceName(fileSolomon.begin(), fileSolomon.end() - string(".txt").length());

	string folderInputESPPRC = "data//ESPPRC//input//";
	string fileInputESPPRC = folderInputESPPRC + instanceName + ".txt";

	string folderOutputESPPRC = "data//ESPPRC//output//";
	string fileOutputESPPRC = folderOutputESPPRC + instanceName + ".txt";
	
	readDataSolomonESPPRC(inst, data, coefDist, prize, precision);
	
	writeToFile(data, fileInputESPPRC);

	readFromFile(data, fileInputESPPRC);

	// Algorithm.
	data.preprocess();
	Data_Auxiliary_ESPPRC auxiliary;
	auto result = DPAlgorithmESPPRC(data, auxiliary, cout);
	printResultsDPAlgorithmESPPRC(data, auxiliary, cout, result);

	return 0;
}

