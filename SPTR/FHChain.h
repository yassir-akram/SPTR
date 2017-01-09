#pragma once

template <typename T> class FHChain
{
public:
	~FHChain();
	FHChain(T v, int p) : v(v), p(p), next(nullptr), father(nullptr), childs(nullptr), deg(0), marked(false) {}
	FHChain(T v, int p, FHChain *prev, FHChain *next) : v(v), p(p), prev(prev), next(next), father(nullptr), childs(nullptr), deg(0), marked(false) {}
	void unlink(FHChain *&from);
	void insert(FHChain *&in);
	void insertaslist(FHChain *&in);

	T v;
	FHChain *prev, *next;
	FHChain *father;
	FHChain *childs;
	unsigned int p;
	int deg;
	bool marked;
};


