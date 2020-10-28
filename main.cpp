#include"Header.h"
#include"general.h"
#include"ESPPRC.h"


int main(int argc, char** argv) {
	string folderSolomon = "data//solomon instances//solomon_100//";
	string folderInstance = "data//ESPPRC//Test//input//";
	string folderOutput = "data//ESPPRC//Test//output//";

	testDPAlgorithmESPPRCFolder(folderSolomon, folderInstance, folderOutput);

/*
	string folderSolomon = "data//solomon instances//solomon_100//";
	string folderInstance = "data//ESPPRC//Test_20201027//input//";
	string folderOutput = "data//ESPPRC//Test_20201027//output//";
	string fileSolomon = "C104.txt";
	string file = fileSolomon;
	pair<bool, bool> dmn = { false,false };
	int prz = 10;

	ParameterTestDPAlgorithmESPPRC parameter;
	parameter.strInputSolomon = folderSolomon + fileSolomon;
	parameter.strInstance = folderInstance + file;
	parameter.strOutput = folderOutput + file;
	parameter.prize = prz;
	parameter.dominateUninserted = dmn.first;
	parameter.dominateInserted = dmn.second;
	testDPAlgorithmESPPRC(parameter, cout);
*/

	return 0;
}

