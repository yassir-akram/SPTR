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

struct Vertex
{

	unsigned int id;
	int lat, lon;

	int t;
	bool computed;
	Chain<struct Arc> *neighbors;
	FHChain<Vertex*> *myFHc;

	//II
	bool IIed;


	Vertex(int id, int lat, int lon) : id(id), lat(lat), lon(lon), computed(false), neighbors(nullptr), myFHc(nullptr), t(0), IIed(false) {}
	Vertex() {}

	~Vertex();
};

typedef struct Arc Arc;
struct Arc
{
	Vertex *to;
	int t;

	Arc(Vertex *to, int t) : to(to), t(t) {};
};

class RoadNetwork
{
public:
	int n, m;
	RoadNetwork() : n(0), m(0), ht(&hashCode, 1024) { };
	//~RoadNetwork();
	void readfromfile(const char* file, float latsr, float lonsr);
	void addVertex(unsigned int id, int lat, int lon);
	bool addArc(unsigned int frid, unsigned int toid, int t);
	int Dijkstra(Vertex *sr);
	void printinfile(const char* file);
	static float distang(float lata, float lona, float latb, float lonb);
	int interpolation(int c1, int c2, int t1, int t2);

	Vertex *select_first_vertex();
	Vertex *select_vertex_rand();
	Vertex *select_vertex_id(int id);
	Vertex *select_vertex_coords(int lat, int lon);

	;
private:
	Vertex *sr;
	Hashtable<struct Vertex, unsigned int> ht;
	static int hashCode(unsigned int n, int N);
};

