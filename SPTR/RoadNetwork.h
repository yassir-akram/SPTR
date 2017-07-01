#pragma once

#include "Hashtable.h"
#include "FibonacciHeap.h"

#include "time.h"

//for stoul
#include <sstream>

// for mmap:
#include "mman.h"
//#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include <ios>

#include <vector>

struct Vertex
{
	FHChain<struct Vertex*> *myFHc;
	Chain<struct Arc> *neighbors;
	Chain<struct Arc> *predecessors;
	Chain<struct Vertex*> *prec, *precinv;
	unsigned int id;
	unsigned int t, tinv, t2;	// The time it takes to get from the source to this point
	int lat, lon;
	bool computed;
	bool stopon;
	
	Vertex(int id, int lat, int lon) : id(id), lat(lat), lon(lon), computed(false), neighbors(nullptr), predecessors(nullptr), myFHc(nullptr), t(0), tinv(0), t2(0), prec(nullptr), precinv(nullptr), stopon(false) {}
	Vertex() {}

	~Vertex();
};

typedef struct Arc Arc;
struct Arc
{
	Vertex *to;
	unsigned int t;

	Arc(Vertex *to, unsigned int t) : to(to), t(t) {};
	Arc() {};
};

class RoadNetwork
{
public:
	~RoadNetwork();
	int n, m;
	// number of Vertices in the Connected component
	int ninCC, ninCCinv;
	// To plot the isochrone :
	unsigned int targetTime;
	unsigned int targetTimeHigh;
	// Sorted Vertex Vector
	Vertex **SV, **SVinv;
	// Maximun time between two vertices
	unsigned int maxt;
	// To show the points that are exactly at targetTime, but on a way to a point at least at targetTimeHigh
	bool III;
	RoadNetwork() : n(0), m(0), ht(&hashCode, 1024), targetTime(0), targetTimeHigh(0), III(false), maxt(0), ninCC(-1), ninCCinv(-1), SV(nullptr), SVinv(nullptr) { };
	//~RoadNetwork();
	void readfromfile(const char* file);
	void addVertex(unsigned int id, int lat, int lon);
	bool addArc(unsigned int frid, unsigned int toid, unsigned int t);
	bool Dijkstra(Vertex *sr, unsigned int Vertex::* t, Chain<struct Arc>* Vertex::* neighs, Chain<struct Vertex*>* Vertex::* prec, Vertex**& SV, int& ninCC, bool main, unsigned int TTH);
	//void printinfile(const char* file);
	//void printroadto(Vertex *to, const char* file);
	static float distang(float lata, float lona, float lats, float lonb);
	//int interpolation(int c1, int c2, int t1, int t2);
	Chain<struct Vertex*>* computeToutSin(unsigned int TT, unsigned int TTH, Vertex **&SV, int& ninCC, unsigned int Vertex::* t, Chain<struct Vertex*>* Vertex::* prec);
	int ToutSinrec(unsigned int& TT, unsigned int& TTH, Vertex* v, Chain<struct Vertex*>*& Vs, unsigned int Vertex::* t, Chain<struct Vertex*>* Vertex::* prec);
	Chain<struct Vertex*>* computeTout(unsigned int TT, unsigned int TTH);
	Chain<struct Vertex*>* computeSin(unsigned int TT, unsigned int TTH);
	unsigned int Reach(Vertex* v);
	unsigned int ReachApprox(Vertex* v);
	void printReach(const char* file, int nb);

	Vertex *select_first_vertex();
	Vertex *select_vertex_rand();
	Vertex *select_vertex_id(int id);
	Vertex *select_vertex_coords(int lat, int lon);
	Vertex *select_vertex_nearest(float lat, float lon);

	void clean_Vertices();
private:
	Vertex *sr;
	Hashtable<struct Vertex, unsigned int> ht;
	static int hashCode(unsigned int n, int N);
};