#pragma once
#define phi (1 + sqrt(5)) / 2

#include "FHChain.h"

template <typename T> class FibonacciHeap
{
public:
	int n;
	FibonacciHeap() : n(0), nodes(nullptr), nodeDeg(nullptr), maxnodesnb(0) {}
	~FibonacciHeap();
	FHChain<T*> *add(T*& el, unsigned int p);
	bool isEmpty();
	T *ext_min();
	void set_pr(FHChain<T*> *&cel, unsigned int &p);
	void generate_nodeDeg(int size);
	int maxnodesnb;
	FHChain<T*> *minnode;
	void print();
private:
	FHChain<T*> *nodes;
	FHChain<T*> **nodeDeg;
	int nbnodes;
	void print(FHChain<T*> *d, int i);
	static void clean(FHChain<T*> *d);
};

