#include"Header.h"
#include"general.h"
#include"ESPPRC.h"


double testConstrainDPAlgorithmESPPRC(const ParameterTestDPAlgorithmESPPRC &parameter, const vector<bool> &cnst, ostream &osAll);
void testConstrainDPAlgorithmESPPRCFolder(const string &folderSolomon, const string &folderInstance, const string &folderOutput);
void TestConstrainFolder();
void TestFolder();


int main(int argc, char** argv) {
	TestFolder();
	return 0;
}


void TestConstrainFolder() {
	string folderSolomon = "data//ESPPRC//Test//solomon instances//solomon_100//";
	string folderInstance = "data//ESPPRC//Test//input//";
	string folderOutput = "data//ESPPRC//Test//output//";

	testConstrainDPAlgorithmESPPRCFolder(folderSolomon, folderInstance, folderOutput);
}


void TestFolder() {
	string folderSolomon = "data//ESPPRC//Test//solomon instances//solomon_100//";
	string folderInstance = "data//ESPPRC//Test//input//";
	string folderOutput = "data//ESPPRC//Test//output//";

	testDPAlgorithmESPPRCFolder(folderSolomon, folderInstance, folderOutput);
}


// Test.
double testConstrainDPAlgorithmESPPRC(const ParameterTestDPAlgorithmESPPRC &parameter, const vector<bool> &cnst, ostream &osAll) {
	Data_Auxiliary_ESPPRC auxiliary;
	try {
		Instance_Solomon inst = readSolomonInstance(parameter.strInputSolomon);

		Data_Input_ESPPRC data;
		data.dominateUninserted = parameter.dominateUninserted;
		data.dominateInserted = parameter.dominateInserted;
		vector<double> prize(inst.vertices.size(), parameter.prize);
		prize[0] = 0;
		int precision = 1;

		double coefDist = 0.2;
		data.sizeQuantLB = 40;
		data.sizeDistLB = 40;
		data.sizeTimeLB = 20;
		data.mustOptimal = true;
		data.minRunTime = 0;
		data.maxDominanceTime = 300;
		data.maxRunTime = 3600;
		data.maxNumCandidates = 2e7;
		data.maxReducedCost = 0;
		data.maxNumRoutesReturned = 10;
		data.maxNumPotentialEachStep = 1e4;
		data.allowPrintLog = true;
		data.constrainResource = cnst;
		data.applyLB = data.constrainResource;

		readDataSolomonESPPRC(inst, data, coefDist, prize, precision);

		writeToFile(data, parameter.strInstance);
		readFromFile(data, parameter.strInstance);

		data.incrementQuantLB = floor((data.QuantityWindow[0].second - data.QuantityWindow[0].first) / data.sizeQuantLB);
		data.incrementDistLB = floor((data.DistanceWindow[0].second - data.DistanceWindow[0].first) / data.sizeDistLB);
		data.incrementTimeLB = floor((data.TimeWindow[0].second - data.TimeWindow[0].first) / data.sizeTimeLB);

		ofstream osOne(parameter.strOutput);
		if (!osOne) throw exception("Failed file operator.");

		// Algorithm.
		data.preprocess();
		auto result = DPAlgorithmESPPRC(data, auxiliary, osOne);
		printResultsDPAlgorithmESPPRC(data, auxiliary, osOne, result);
		osOne.close();

		// Output.
		osAll << data.name << '\t' << parameter.prize << '\t' << data.dominateUninserted << '\t' << data.dominateInserted << '\t'
			<< data.NumVertices << '\t' << data.numArcs << '\t' << data.density << '\t' << data.numNegArcs << '\t'
			<< data.percentNegArcs << '\t' << auxiliary.timeLB << '\t' << auxiliary.timeUB << '\t' << auxiliary.timeDP << '\t'
			<< auxiliary.numUnGeneratedLabelsInfeasibility << '\t' << auxiliary.numGeneratedLabels << '\t'
			<< auxiliary.numPrunedLabelsBound << '\t' << auxiliary.numUnInsertedLabelsDominance << '\t'
			<< auxiliary.numDeletedLabelsDominance << '\t' << auxiliary.numSavedLabels << '\t' << auxiliary.numCompletedRoutes << endl;
	}
	catch (const exception exc) {
		printErrorAndExit("testConstrainDPAlgorithmESPPRC", exc);
	}
	return runTime(auxiliary.startTime);
}


// Test all instances under a folder.
void testConstrainDPAlgorithmESPPRCFolder(const string &folderSolomon, const string &folderInstance, const string &folderOutput) {
	try {
		ParameterTestDPAlgorithmESPPRC parameter;
		parameter.prize = 5;
		parameter.dominateUninserted = true;
		parameter.dominateInserted = true;
		vector<bool> TF = { true,false };
		vector<string> names;
		getFiles(folderSolomon, vector<string>(), names);
		for (const auto &fileSolomon : names) {
			for (const auto &Q : TF) {
				for (const auto &D : TF) {
					vector<bool> cnst = { Q,D,true };
					string file = string(fileSolomon.begin(), fileSolomon.end() - 4) + "_" + numToStr(cnst[0]) + "_"
						+ numToStr(cnst[1]) + "_" + numToStr(cnst[2]) + ".txt";
					parameter.strInputSolomon = folderSolomon + fileSolomon;
					parameter.strInstance = folderInstance + file;
					parameter.strOutput = folderOutput + file;

					testConstrainDPAlgorithmESPPRC(parameter, cnst, cout);
				}
			}
		}
	}
	catch (const exception &exc) {
		printErrorAndExit("testDPAlgorithmESPPRCFolder", exc);
	}
}

