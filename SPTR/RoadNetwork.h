#pragma once

#include "Chain.h"
#include "DLChain.h"
#include "Hashtable.h"
#include "FibonacciHeap.h"

struct Vertex
{
	unsigned int id;
	int lat, lon;

	int t;
	bool computed;
	Chain<struct Arc> *neighbors;
	DLChain<Vertex*> *me;
	DLChain<Vertex*> *FHfather;
	DLChain<Vertex*> *FHchilds;
	int deg;
	bool marked;

	Vertex(int id, int lat, int lon) : id(id), lat(lat), lon(lon), computed(false), neighbors(nullptr), FHfather(nullptr), FHchilds(nullptr), deg(0), marked(false), me(nullptr) {}
	Vertex() {}
};

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
	void addV(unsigned int id, int lat, int lon);
	bool addA(unsigned int frid, unsigned int toid, int t);
	void Dijkstra(Vertex *v);
	Vertex *select_first();
private:
	Hashtable<Vertex, unsigned int> ht;
	static int hashCode(unsigned int n, int N);
};

