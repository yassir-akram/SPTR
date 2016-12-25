// SPTR.cpp : définit le point d'entrée pour l'application console.
//

#include "stdafx.h"

#include "math.h"
#include "RoadNetwork.h"
#include "Hashtable.h"

using namespace std;

int main()
{
	const char * rfile = "C:\\Users\\Yassir\\Downloads\\RoadNetworks\\data\\France.in",
				* pfile = "C:\\Users\\Yassir\\Downloads\\RoadNetworks\\vis\\points.js";
	RoadNetwork rn;
	rn.readfromfile(rfile, (float)48.848096, (float)2.344330);
	if (rn.Dijkstra(rn.select_vertex_id(470134)))
	{
		system("PAUSE");
		return 1;
	}
	rn.printinfile(pfile);
	std::cout << "Done!" << endl;
	
	system("PAUSE");
	return EXIT_SUCCESS;
}


