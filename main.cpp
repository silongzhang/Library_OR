#include"Header.h"
#include"general.h"
#include"ESPPRC.h"


int main(int argc, char** argv) {
	string folderSolomon = "data//solomon instances//solomon_100//";
	string folderInstance = "data//ESPPRC//Test_20201027//input//";
	string folderOutput = "data//ESPPRC//Test_20201027//output//";

	testDPAlgorithmESPPRCFolder(folderSolomon, folderInstance, folderOutput);
	return 0;
}

