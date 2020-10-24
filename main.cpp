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

	string instanceName(fileSolomon.begin(), fileSolomon.end() - string(".txt").length());

	string folderInputESPPRC = "data//ESPPRC//input//";
	string fileInputESPPRC = folderInputESPPRC + instanceName + ".txt";

	string folderOutputESPPRC = "data//ESPPRC//output//";
	string fileOutputESPPRC = folderOutputESPPRC + instanceName + ".txt";

	Instance_Solomon inst = readSolomonInstance(fileInputSolomon);
	
	Data_Input_ESPPRC data;
	double coefDist = 0.2;
	double prz = 30;
	vector<double> prize(inst.vertices.size(), prz);
	prize[0] = 0;
	int precision = 1;
	readDataSolomonESPPRC(inst, data, coefDist, prize, precision);
	
	writeToFile(data, fileInputESPPRC);

	readFromFile(data, fileInputESPPRC);

	return 0;
}

