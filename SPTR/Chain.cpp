#include "stdafx.h"
#include "Chain.h"

template <typename T> void Chain<T>::deleterec()
{
	Chain<T> *c = this, *next;
	do
	{
		next = c->next;
		delete c;
		c = next;
	} while (c != nullptr);
}

template class Chain<struct Vertex*>;
template class Chain<struct Arc>;
template class Chain<int>;