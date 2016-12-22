#pragma once

template <class T>
class DLChain
{
public:
	DLChain(T v) : v(v), next(nullptr) {}
	DLChain(T v, DLChain *prev, DLChain *next) : v(v), prev(prev), next(next) {}

	T v;
	DLChain *prev, *next;
};

