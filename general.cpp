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


// Read Solomon instance.
Instance_Solomon readSolomonInstance(const string &input) {
	Instance_Solomon result;
	try {
		ifstream ins(input);
		if (!ins) throw exception("Input file Error.");

		ins >> result.name;

		string temp;
		// Skip four lines.
		getline(ins, temp);
		getline(ins, temp);
		getline(ins, temp);
		getline(ins, temp);

		ins >> result.numVehicle >> result.capacity;

		// Skip five lines.
		getline(ins, temp);
		getline(ins, temp);
		getline(ins, temp);
		getline(ins, temp);
		getline(ins, temp);

		int iter = -1;
		Customer_Solomon cust;
		for (ins >> cust.id; !ins.eof(); ins >> cust.id) {
			if (++iter != cust.id) throw exception("Input file Error.");
			ins >> cust.xCoord >> cust.yCoord >> cust.demand >> cust.readyTime >> cust.dueTime >> cust.serviceTime;
			result.vertices.push_back(cust);
		}
		ins.close();
	}
	catch (const exception &exc) {
		printErrorAndExit("readSolomonInstance", exc);
	}
	return result;
}

