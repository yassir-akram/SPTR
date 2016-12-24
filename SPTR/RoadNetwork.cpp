#include "stdafx.h"
#include "RoadNetwork.h"

#define II false
#define targetTime 2*3600*1000

//RoadNetwork::~RoadNetwork()
//{
//}
using namespace std;

void RoadNetwork::readfromfile(const char* file, float latsr, float lonsr)
{
	time_t tb, te;

	std::cout << "Reading from file " << file << endl;
	ifstream myfile;

	unsigned int id, p1, p2;
	int lon, lat, t;

	myfile.open(file);
	if (myfile.is_open())
	{
		time(&tb);
		while (myfile.get() == 'v')
		{
			myfile >> id >> lon >> lat;
			if (distang((float)lat / 1000000, (float)lon / 1000000, latsr, lonsr) <= 130.*2./6371.)
				addVertex(id, lat, lon);
			myfile.get();
		}
		myfile.unget();
		time(&te);
		std::cout << n << " vertexes added." << endl;
		std::cout << "Ellapsed time: " << difftime(te, tb) << "s" << endl;

		time(&tb);
		while (myfile.get() == 'a')
		{
			myfile >> p1 >> p2 >> t;
			addArc(p1, p2, t);
			myfile.get();
		}
		time(&te);
		std::cout << m << " arc added." << endl;
		std::cout << "Ellapsed time: " << difftime(te, tb) << "s" << endl;
	}
	myfile.close();
}

void RoadNetwork::addVertex(unsigned int id, int lat, int lon)
{
	ht.add(new Vertex(id, lat, lon), id);
	n++;
}

bool RoadNetwork::addArc(unsigned int frid, unsigned int toid, int t)
{
	Vertex *fr = ht.find(frid);
	if (fr == nullptr) return false;
	Vertex *to = ht.find(toid);
	if (to == nullptr) return false;
	fr->neighbors = new Chain<Arc>({ to, t }, fr->neighbors);
	m++;
	return true;
}

int RoadNetwork::Dijkstra(Vertex *sr)
{
	if (sr == 0)
	{
		std::cout << "Vertex not found!" << endl;
		return 1;
	}

	time_t tb, te;
	std::cout << "Beguinning Dijkstra:" << endl;
	time(&tb);

	//Cleaning from last executiion
	KeyList<struct Vertex, unsigned int> *E = ht.E;
	for (int N = ht.N; N--; E++)
		for (Chain<struct Entry<struct Vertex, unsigned int>*> *c = E->first; c != nullptr; c = c->next)
			c->var->value->computed = false;

	this->sr = sr;
	sr->t = 0;
	sr->computed = true;
	sr->IIed = false;

	FibonacciHeap<struct Vertex> fh;

	//Adding first neighbrs
	Chain<Arc> *c = sr->neighbors;
	while (c != nullptr)
	{
		fh.add(c->var.to, c->var.t);
		c = c->next;
	}

	//Generating nodeDeg
	fh.generate_nodeDeg(ht.n);

	//Main loop
	while (!fh.isEmpty())
	{
		Vertex *vmin = fh.ext_min();

		// Si on est déjà en dehors du cadre de nos recherches, on peut stopper l'exploration
		if (vmin->t > (targetTime + (!II ? 60000 : 0)))
			break;

		vmin->computed = true;
 		
		//Computing neighbrs
		Chain<Arc> *c = vmin->neighbors;
		while (c != nullptr)
		{
			Vertex *to = c->var.to;
			//if already in the heap
			if (to->myFHc == nullptr) fh.add(to, vmin->t + c->var.t);
			else if (!c->var.to->computed)
			{
				int tu = vmin->t + c->var.t;
				if (tu < to->t)	fh.set_pr(to, tu);
			}
#if II
			if (vmin->t < targetTime && to->t >= targetTime)
			{
				to->IIed = true;
				to->lat = interpolation(vmin->lat, to->lat, vmin->t, to->t);
				to->lon = interpolation(vmin->lon, to->lon, vmin->t, to->t);
			}
			else to->IIed = false;
#endif
			c = c->next;
		}
	}

	time(&te);
	std::cout << "Dijkstra ended sucessfully!" << endl;
	std::cout << "Ellapsed time: " << difftime(te, tb) << "s" << endl;

	return 0;
}

int RoadNetwork::hashCode(unsigned int n, int N)
{
	return n%N;
}

Vertex *RoadNetwork::select_first_vertex()
{
	KeyList<struct Vertex, unsigned int> *L = ht.E;
	while (!L->n) L++;
	return L->first->var->value;
}

Vertex *RoadNetwork::select_vertex_rand()
{
	std::srand((int)time(nullptr));
	int r = std::rand() % ht.N;
	KeyList<struct Vertex, unsigned int> *L = ht.E+r;
	while (!L->n) L++;
	return L->first->var->value;
}

Vertex *RoadNetwork::select_vertex_id(int id)
{
	return ht.find(id);
}

Vertex *RoadNetwork::select_vertex_coords(int lat, int lon)
{
	KeyList<Vertex, unsigned int> *E = ht.E;
	for (int N = ht.N; N--; E++)
	{
		for (Chain<struct Entry<Vertex, unsigned int>*> *c = E->first; c != nullptr; c = c->next)
		{
			Vertex *u = c->var->value;
			if (u->lat == lat && u->lon == lon)
				return u;
		}
	}
	return nullptr;

}

void RoadNetwork::printinfile(const char* file)
{
	std::cout << "Exporting points to file " << file << endl;
	ofstream myfile;
	myfile.open(file);
	myfile << endl << "var plottedPoints = [" << endl;
	KeyList<struct Vertex, unsigned int> *E = ht.E;
	for (int N = ht.N; N--; E++)
	{
		for (Chain<struct Entry<struct Vertex, unsigned int>*> *c = E->first; c != nullptr; c = c->next)
		{
			Vertex *u = c->var->value;
			if (!II && targetTime - 60000 < u->t && u->t < targetTime + 60000 || II && u->IIed)
				myfile << "\t[" << (float)u->lat / 1000000 << "," << (float)u->lon / 1000000 << "]," << endl;
		}
	}
	myfile << "];" << endl;
	myfile << endl << "var centralMarker =" << endl;
	myfile << "\t[" << (float)sr->lat / 1000000 << "," << (float)sr->lon / 1000000 << "]" << endl;
	myfile << ";" << endl;
	myfile.close();
}

float RoadNetwork::distang(float lata, float lona, float latb, float lonb)
{
#define PI 3.141592
	return (float)acos(sin(lata*PI / 180)*sin(latb*PI / 180) + cos(lata*PI / 180)*cos(latb*PI / 180)*cos((lonb - lona)*PI / 180));
}

int RoadNetwork::interpolation(int c1, int c2, int t1, int t2)
{
	float vitesse = (float)(c2 - c1) / (float)(t2 - t1);
	float expectedPoint = c1 + vitesse * (targetTime - t1);
	return (int)expectedPoint;
}