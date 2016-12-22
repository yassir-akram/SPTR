// SPTR.cpp : définit le point d'entrée pour l'application console.
//

#include "stdafx.h"

#include "RoadNetwork.h"
#include "Hashtable.h"

using namespace std;


int hashCode(std::string mystr, int N);
int hashCodeui(unsigned int n, int N);

int main()
{

	RoadNetwork rn;

	ifstream myfile;
	myfile.open("C:\\Users\\Yassir\\Downloads\\RoadNetworks\\data\\france.in");
	
	unsigned id;
	int lat, lon;
	unsigned int p1, p2;
	int t;

	if (myfile.is_open())
	{
		do
		{
			switch (myfile.get())
			{
			case 'v':
				myfile >> id >> lat >> lon;
				rn.addV(id, lat, lon);
				break;
			case 'a':
				myfile >> p1 >> p2 >> t;
				rn.addA(p1, p2, t);
				break;
			}
			myfile.get();
		} while (!myfile.eof());
	}
	myfile.close();
	
	rn.Dijkstra(rn.select_first());

	system("PAUSE");
	return EXIT_SUCCESS;
}

int hashCode(std::string mystr, int N)
{
	int h = 0;
	int n = mystr.size();
	while (--n) h = (h + (mystr)[n]) % N;
	return h;
}

int hashCodeui(unsigned int n, int N)
{
	return n%N;
}


