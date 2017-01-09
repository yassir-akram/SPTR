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


	std::srand((int)time(nullptr));
	RoadNetwork *rn = new RoadNetwork;
	rn->readfromfile(rfile);
	rn->Reach(rn->select_vertex_id(830473852));
	//rn->printReach(pfile, 1000);
	delete rn;
	return EXIT_SUCCESS;
}

