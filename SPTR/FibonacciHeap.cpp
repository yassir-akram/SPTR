#include "stdafx.h"
#include "FibonacciHeap.h"


//FibonacciHeap::~FibonacciHeap()
//{
//}


template <typename T> void FibonacciHeap<T>::add(T* el, int p)
{
	if (nodes == nullptr) 
	{
		el->me = new DLChain<T*>(el);
		nodes = el->me;
		nodes->prev = nodes;
		nodes->next = nodes;

		minnode = nodes;
	}
	else
	{
		el->me = new DLChain<T*>(el, nodes->prev, nodes);
		nodes->prev->next = el->me;
		nodes->prev = el->me;

		if (el->t < minnode->v->t) minnode = el->me;
	}
	n++;
	nbnodes++;
}

template <typename T> void FibonacciHeap<T>::generate_nodedeg()
{
	maxnodesnb = (int)(log(sqrt(5)*n) / log(phi))+50;
	if (nodeDeg != nullptr) delete[] nodeDeg;
	nodeDeg = new DLChain<T*>*[maxnodesnb];
}

template <typename T> bool FibonacciHeap<T>::isEmpty()
{
	return nodes == nullptr;
}

template <typename T> void FibonacciHeap<T>::merge(DLChain<T*> *L1, DLChain<T*> *L2)
{
	L1->prev->next = L2;
	L2->prev->next = L1;
	DLChain<T*> *tmp = L1->prev;
	L1->prev = L2->prev;
	L2->prev = tmp;
}

template <typename T> void FibonacciHeap<T>::insert(DLChain<T*> *c, DLChain<T*> *L)
{
	//couper c de sa chaine
	c->next->prev = c->prev;
	c->prev->next = c->next;

	//inserer c dans L
	c->prev = L->prev;
	c->next = L;
	L->prev->next = c;
	L->prev = c;
}

Vertex *FibonacciHeap<struct Vertex>::ext_min(Hashtable<Vertex, unsigned int> ht)
{
	//Clean nodeDeg
	for (int i = 0; i < maxnodesnb; i++)  nodeDeg[i] = nullptr;
	
	//Compute min
	//Add min_childs
	Vertex *minV = minnode->v;
	nbnodes--;
	if (minnode->next == minnode)
	{
		nodes = minnode->v->FHchilds;
	}
	else 
	{
		minnode->prev->next = minnode->next;
		minnode->next->prev = minnode->prev;
		nodes = nodes->next;
		
	}

	DLChain<Vertex*> *d = minnode->v->FHchilds, *c = d;
	if (d != nullptr)
	{
		do
		{
			nbnodes++; c->v->FHfather = nullptr;
			c = c->next;
		} while (c != d);
	}

	if (minnode->next != minnode) if (minnode->v->FHchilds != nullptr) merge(nodes, minnode->v->FHchilds);


	



	//Clean (unique degree)
	int nbnodes_pr = nbnodes;
	minnode = nodes;
	DLChain<Vertex*> *next;
	for (DLChain<Vertex*> *c = nodes; nbnodes_pr--; c = next)
	{
 		if (c->v->t < minnode->v->t)
			minnode = c;

		next = c->next;

		while (nodeDeg[c->v->deg] != nullptr) 
		{
			DLChain<Vertex*> *cd = nodeDeg[c->v->deg];
			
			//cd de plus petite value
			if (cd->v->t > c->v->t)
			{
				nodeDeg[c->v->deg] = c;
				c = cd;
				cd = nodeDeg[c->v->deg];
			}
			if (minnode == c) minnode = cd;
			//déref
			nodeDeg[c->v->deg] = nullptr;

			//au cas ou nodes devienne fils
			nodes = cd;
			if (cd->v->FHchilds == nullptr)
			{
				cd->v->FHchilds = c;
				c->prev->next = c->next;
				c->next->prev = c->prev;
				c->next = c;
				c->prev = c;
			}
			else insert(c, cd->v->FHchilds);
			c->v->FHfather = cd;

			c->v->marked = false;
			cd->v->deg++;

			nbnodes--;
			c = cd;
			
		}
		nodeDeg[c->v->deg] = c;

		//print();
	}
	

	return minV;
}

void FibonacciHeap<struct Vertex>::set_pr(Vertex *el, int p)
{
	el->t = p;
	if (el->t < minnode->v->t) minnode = el->me;

	if (el->FHfather != nullptr && el->FHfather->v->t > el->t)
	{
		DLChain<Vertex*> *f;

		do
		{
			el->marked = false;
			if (el->me->next == el->me) el->FHfather->v->FHchilds = nullptr;
			else el->FHfather->v->FHchilds = el->me->next;
			insert(el->me, nodes);
			nbnodes++;
			f = el->FHfather;
			el->FHfather = nullptr;
			if (f->v->deg == el->deg+1) f->v->deg--;

			if (!f->v->marked) { f->v->marked = true; break; };

			el = f->v;
		} while (el->FHfather != nullptr);
	}
}

//void FibonacciHeap<struct Vertex>::print(DLChain<Vertex*> *d, int i)
//{
//	if (d == nullptr) return;
//	DLChain<Vertex*> *c = d;
//	do
//	{
//		for (int k = i; k; k--)  std::cout << '\t';
//		std::cout << c->v->id << ":" << c->v->t << std::endl;
//		print(c->v->FHchilds, i + 1);
//	} while ((c = c->next) != d);
//
//
//}
//
//template<> void FibonacciHeap<struct Vertex>::print()
//{
//	print(nodes, 0);
//	std::cout << std::endl;
//	system("PAUSE");
//}

//template<> void FibonacciHeap<struct Vertex>::test()
//{
//	int nb = nbnodes;
//	int nb2 = 0;
//	bool found = false;
//	DLChain<Vertex*> *d = nodes, *c = d;
//	if (d != nullptr)
//	{
//		do
//		{
//			nb2++;
//			if (c == minnode) found = true;
//			c = c->next;
//		} while (c != d);
//	}
//	if (n==3297)
//		system("PAUSE");
//	if (nb!=nb2 || !found)
//		system("PAUSE");
//}