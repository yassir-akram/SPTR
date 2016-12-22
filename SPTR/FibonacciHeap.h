#pragma once
#define phi (1 + sqrt(5)) / 2

#include "RoadNetwork.h"
#include "DLChain.h"

template <typename T> class FibonacciHeap
{
public:
	int n;
	FibonacciHeap() : n(0), nodes(nullptr), nodeDeg(nullptr), maxnodesnb(0) {}
	//~FibonacciHeap();
	void add(T* el, int p);
	bool isEmpty();
	T *ext_min(Hashtable<Vertex, unsigned int> ht);
	void set_pr(T *el, int p);
	void generate_nodedeg();
	int maxnodesnb;
	//void print(DLChain<T*> *d, int i);
	//void print();
	//void test();
private:
	DLChain<T*> *nodes;
	DLChain<T*> *minnode;
	DLChain<T*> **nodeDeg;
	int nbnodes;

	static void merge(DLChain<T*> *L1, DLChain<T*> *L2);
	static void insert(DLChain<T*> *c, DLChain<T*> *L);
};

template class FibonacciHeap<struct Vertex>;