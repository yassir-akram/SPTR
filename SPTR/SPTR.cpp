// SPTR.cpp : définit le point d'entrée pour l'application console.
//

#include "stdafx.h"

#include "math.h"

using namespace std;

int main()
{
	const char * rfile = "C:\\Users\\Yassir\\Downloads\\RoadNetworks\\data\\France.in",
				* pfile = "C:\\Users\\Yassir\\Downloads\\RoadNetworks\\vis\\points.js";
	
	RoadNetwork *rn = new RoadNetwork;
	rn->readfromfile(rfile, (float)48.848096, (float)2.344330);
	Vertex *sr = rn->select_vertex_id(470134);
	if (sr != nullptr)
	{
		rn->Dijkstra(sr);
		rn->printinfile(pfile);
		std::cout << "Done!" << endl;
	}
	delete rn;

	system("PAUSE");
	return EXIT_SUCCESS;
}


