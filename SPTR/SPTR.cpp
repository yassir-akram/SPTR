// SPTR.cpp : définit le point d'entrée pour l'application console.
//

#include "stdafx.h"

#include "math.h"

using namespace std;

int main(int argc, char *argv[])
{
	if (argc == 1) return 255;
	const char * rfile = argv[1],	// Input data file
		*pfile = "reaches.csv";		// Output file

	// Do we compute the reach of a random sample
	bool sample = false;
	// Size of the said sample
	int sampleSize;

	// ID of the vertex whose reach is to be computed
	bool singleID = true;
	int vertexId= 470134;

	// LAt and Lon of the vertex whose reach is to be computed
	bool singleCoords = false;
	unsigned int vertexLon = 2343656, vertexLat = 48847363;


	/*
	Reading the options :
	-sample n : computes the reach value of a random sample of size n
	-vertexId n : computes the reach value of the vertex whose id is n
	-vertexLat n : sets the lat of the vertex whose reach is to be computed on n
	-vertexLon n : sets the lon of the vertex whose reach is to be computed on n
	*/
	for (int i = 2; i < argc; i++)
	{
		if (!strcmp(argv[i], "-sample"))
		{
			sample = true;
			singleID = false;
			singleCoords = false;
			sampleSize = stoi(argv[i + 1]);
			i++;
		}
		else if (!strcmp(argv[i], "-vertexId"))
		{
			sample = false;
			singleID = true;
			singleCoords = false;
			vertexId = stoi(argv[i + 1]);
			i++;
		}
		else if (!strcmp(argv[i], "-vertexLat"))
		{
			sample = false;
			singleID = false;
			singleCoords = true;
			vertexLat = stoi(argv[i + 1]);
			i++;
		}
		else if (!strcmp(argv[i], "-vertexLon"))
		{
			sample = false;
			singleID = false;
			singleCoords = true;
			vertexLon = stoi(argv[i + 1]);
			i++;
		}
		else
		{
			cout << "\n  reach inputFile [-sample:sample size] | [-vertexId:ID] | [-vertexLat:Lat] [-vertexLon:Lon]\n\n"
				<< "\t-sample n : computes the reach value of a random sample of size n\n"
				<< "\t-vertexId n : computes the reach value of the vertex whose id is n\n"
				<< "\t-vertexLat n : sets the lat of the vertex whose reach is to be computed on n (integer, 48.848096 becomes 48848096)\n"
				<< "\t-vertexLon n : sets the lon of the vertex whose reach is to be computed on n\n"
				<< "\t\tNote that if there is no vertex with the given coordinates, the closest one will be taken as a starting point.\n"
				<< "\t\tTherefore it is not compulsory to bother giving the coordinates of an existing point.\n";
			return 255;
		}
	}
	// Seeding the random generator
	std::srand((int)time(nullptr));

	// Creating our main object
	RoadNetwork *rn = new RoadNetwork;

	// Loading the input data file
	rn->readfromfile(rfile);

	// Executing the task, depending on what is to be computed :

	// Compute the reach of vertex designed by its id
	if(singleID)
	{
		unsigned int r;
		Vertex* v = rn->select_vertex_id(vertexId);
		if (v == nullptr)
			std::cout << "No vertex with id " << vertexId << endl;
		else
		{
			r = rn->Reach(v);
			std::cout << "The approx. reach for the vertex id=" << vertexId << " is " << r << endl;
		}
	}
	// Compute the reach of the vertex nearest to a set of coordinates
	else if(singleCoords)
	{
		unsigned int r;
		Vertex* v = rn->select_vertex_nearest((float)vertexLat / 1000000, (float)vertexLon / 1000000);
		if (v == nullptr)
			std::cout << "No vertex near coordinates lat=" << vertexLat << ", lon=" << vertexLon << endl;
		else
		{
			r = rn->Reach(v);
			std::cout << "The approx. reach for the nearest vertex of lat=" << vertexLat << ", lon=" << vertexLon << " is " << r << endl;
		}
	}
	// Compute the reach of a random set of vertex
	else if(sample)
		rn->printReach(pfile, sampleSize);
	delete rn;
	return EXIT_SUCCESS;
}

