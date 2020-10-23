#include"Header.h"
#include"general.h"

extern clock_t start;
extern clock_t last;

int main(int argc, char** argv) {
	start = clock();
	last = start;

	Instance_Solomon elem = readSolomonInstance("data//solomon instances//solomon-100//c101.txt");

	cout << elem.name << endl << elem.numVehicle << '\t' << elem.capacity << endl;

	for (const auto &cust : elem.vertices) {
		cout << cust.id << '\t' << cust.xCoord << '\t' << cust.yCoord << '\t' << cust.demand << '\t' 
			<< cust.readyTime << '\t' << cust.dueTime << '\t' << cust.serviceTime << endl;
	}

	return 0;
}

