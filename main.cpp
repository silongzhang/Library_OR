#include"Header.h"
#include"general.h"
#include"ESPPRC.h"

extern clock_t start;
extern clock_t last;

int main(int argc, char** argv) {
	string folderSolomon = "data//solomon instances//solomon_100//";
	string folderInstance = "data//ESPPRC//Test_20201024//input//";
	string folderOutput = "data//ESPPRC//Test_20201024//output//";

	vector<double> prize = { 10,15,20,25 };
	vector<pair<bool, bool>> dominance = { { false,false },{ false,true },{ true,false },{ true,true } };
	ParameterTestDPAlgorithmESPPRC parameter;

	string fileSolomon = "C101.txt";
	for (const auto &prz : prize) {
		for (const auto &dmn : dominance) {
			string file = numToStr(prz) + "_" + numToStr(dmn.first) + "_" + numToStr(dmn.second) + "_" + fileSolomon;
			parameter.strInputSolomon = folderSolomon + fileSolomon;
			parameter.strInstance = folderInstance + file;
			parameter.strOutput = folderOutput + file;
			parameter.prize = prz;
			parameter.dominateUninserted = dmn.first;
			parameter.dominateInserted = dmn.second;
			testDPAlgorithmESPPRC(parameter, cout);
		}
	}
	return 0;
}

