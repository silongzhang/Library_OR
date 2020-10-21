#include"Header.h"

extern clock_t start;
extern clock_t last;

int main(int argc, char** argv) {
	start = clock();
	last = start;

	return 0;
}

