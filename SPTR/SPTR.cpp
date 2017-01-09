// SPTR.cpp : définit le point d'entrée pour l'application console.
//

#include "stdafx.h"

#include "math.h"

using namespace std;

int main(int argc, char *argv[])
{
	if (argc == 1) return 255;
	const char * rfile = argv[1],	// Input data file
		*pfile = "C:\\Users\\Yassir\\Downloads\\RoadNetworks\\data\\reaches.csv";		// Output file

	RoadNetwork *rn = new RoadNetwork;
	rn->readfromfile(rfile);
	rn->printReach(pfile, 1000);
	delete rn;
	system("PAUSE");
	return EXIT_SUCCESS;
}

