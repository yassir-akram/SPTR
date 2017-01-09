#pragma once


template <class T>
class Chain
{
public:
	Chain() : next(nullptr) {}
	Chain(T var) : var(var), next(nullptr) {}
	Chain(T var, Chain *next) : var(var), next(next) {}

	T var;
	Chain *next;

	void deleterec();
};



