#include "stdafx.h"
#include "FibonacciHeap.h"


template <typename T> FibonacciHeap<T>::~FibonacciHeap()
{
	if (nodeDeg != nullptr) delete[] nodeDeg;
	if (nodes != nullptr) clean(nodes);
}

template <typename T> void FibonacciHeap<T>::clean(FHChain<T*> *d)
{
	FHChain<T*> *c = d;
	FHChain<T*> *next;
	do
	{
		next = c->next;
		if (c->childs != nullptr) clean(c->childs);
		delete c;
		c = next;
	} while (c != d);
}

template <typename T> FHChain<T*> *FibonacciHeap<T>::add(T*& el, unsigned int p)
{
	n++;
	nbnodes++;
	if (nodes == nullptr) 
	{
		nodes = new FHChain<T*>(el, p);
		nodes->prev = nodes;
		nodes->next = nodes;

		minnode = nodes;
		return nodes;
	}
	else
	{
		nodes->prev->next = new FHChain<T*>(el, p, nodes->prev, nodes);
		nodes->prev = nodes->prev->next;

		if (p < minnode->p) minnode = nodes->prev;
		return nodes->prev;
	}
}

template <typename T> void FibonacciHeap<T>::generate_nodeDeg(int size)
{
	maxnodesnb = (int)(log(sqrt(5)*size) / log(phi));
	if (nodeDeg != nullptr) delete[] nodeDeg;
	nodeDeg = new FHChain<T*>*[maxnodesnb];
	for (int i = 0; i < maxnodesnb; i++) nodeDeg[i] = nullptr;
}

template <typename T> bool FibonacciHeap<T>::isEmpty()
{
	return nodes == nullptr;
}



template <typename T> T *FibonacciHeap<T>::ext_min()
{
	//Save
	FHChain<T*> *cmin = minnode;
	T *vmin = minnode->v;

	//Add min childs
	minnode->unlink(nodes);
	nbnodes--;
	n--;

	FHChain<T*> *d = minnode->childs, *c = d;
	if (d != nullptr)
	{
		do
		{
			nbnodes++; c->father = nullptr;
			c = c->next;
		} while (c != d);
	}

	if (minnode->childs != nullptr) minnode->childs->insertaslist(nodes);

	//Clean (unique degree)
	minnode = nodes;
	int nbnodes_pr = nbnodes;
	FHChain<T*> *next;
	for (FHChain<T*> *c = nodes; nbnodes_pr--; c = next)
	{
 		if (c->p < minnode->p)
			minnode = c;

		next = c->next;

		while (nodeDeg[c->deg] != nullptr) 
		{
			FHChain<T*> *cd = nodeDeg[c->deg];
			
			//cd de plus petite value
			if (cd->p > c->p)
			{
				nodeDeg[c->deg] = c;
				c = cd;
				cd = nodeDeg[c->deg];
			}
			if (minnode == c) minnode = cd;

			//déref
			nodeDeg[c->deg] = nullptr;

			c->unlink(nodes);
			c->insert(cd->childs);
			c->father = cd;
			c->marked = false;
			cd->deg++;

			nbnodes--;
			c = cd;
		}
		nodeDeg[c->deg] = c;
	}

	//Clean nodeDeg
	nbnodes_pr = nbnodes;
	for (FHChain<T*> *c = nodes; nbnodes_pr--; c = c->next)	nodeDeg[c->deg] = nullptr;

	
	delete cmin;
	return vmin;
}

template<typename T> void FibonacciHeap<T>::set_pr(FHChain<T*> *&cel, unsigned int& p)
{
	cel->p = p;
	if (cel->p < minnode->p) minnode = cel;

	if (cel->father != nullptr && cel->father->p > cel->p)
	{
		FHChain<T*> *f;

		do
		{
			f = cel->father;
			cel->unlink(f->childs);
			f->deg--;
			cel->father = nullptr;
			cel->insert(nodes);
			nbnodes++;
			if (f->father == nullptr) break;
			if (!f->marked)
			{
				f->marked = true;
				break;
			}
			f->marked = false;
			cel = f;
		} while (true);
	}
}

template<> void FibonacciHeap<struct Vertex>::print(FHChain<struct Vertex*> *d, int i)
{
	if (d == nullptr) return;
	FHChain<struct Vertex*> *c = d;
	do
	{
		for (int k = i; k; k--)  std::cout << '\t';
		std::cout << c->v->id << ":" << c->p << std::endl;
		print(c->childs, i + 1);
	} while ((c = c->next) != d);
}

template<typename T> void FibonacciHeap<T>::print()
{
	std::cout << "nbnodes:" << nbnodes << std::endl;
	print(nodes, 0);
	std::cout << std::endl;
	system("PAUSE");
}

template<> void FibonacciHeap<int>::print(FHChain<int*> *d, int i)
{
	if (d == nullptr) return;
	FHChain<int*> *c = d;
	do
	{
		for (int k = i; k; k--)  std::cout << '\t';
		std::cout << *(c->v) << ":" << c->p << std::endl;
		print(c->childs, i + 1);
	} while ((c = c->next) != d);


}
template class FibonacciHeap<struct Vertex>;
template class FibonacciHeap<int>;