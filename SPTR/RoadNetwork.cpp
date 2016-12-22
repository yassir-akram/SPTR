#include "stdafx.h"
#include "RoadNetwork.h"
#include "KeyList.h"

//RoadNetwork::~RoadNetwork()
//{
//}
using namespace std;

void RoadNetwork::addV(unsigned int id, int lat, int lon)
{
	ht.add(new Vertex(id, lat, lon), id);
	n++;
}

bool RoadNetwork::addA(unsigned int frid, unsigned int toid, int t)
{
	Vertex *fr = ht.find(frid);
	if (fr == nullptr) return false;
	Vertex *to = ht.find(toid);
	if (to == nullptr) return false;
	fr->neighbors = new Chain<Arc>({ to, t }, fr->neighbors);
	m++;
	return true;
}

void RoadNetwork::Dijkstra(Vertex *sr)
{
	cout << ht.mls() << endl;

	sr->t = 0;
	sr->computed = true;

	FibonacciHeap<struct Vertex> fh;

	KeyList<Vertex, unsigned int> *E = ht.E;
	for (int N = ht.N; N-- ; E++)
	{
		for (Chain<struct Entry<Vertex, unsigned int>*> *c = E->first; c!= nullptr; c=c->next)
		{
			Vertex *u = c->var->value;
			if (u != sr) { u->t = INT_MAX;	u->computed = false; }
			fh.add(u, u->t);
		}
	}


	fh.generate_nodedeg();
	int n = 0;
	while (!fh.isEmpty())
	{
		//if (n >= 17964) fh.print();
		//if (n >= 3297)
		//	cout << endl;
		//fh.test();
		Vertex *u = fh.ext_min(ht);
		u->computed = true;
 		Chain<Arc> *c = u->neighbors;
		if (u->t != INT_MAX)
		{
			while (c != nullptr)
			{
				if (!c->var.to->computed)
				{
					int tu = u->t + c->var.t;
					Vertex *v = c->var.to;
					if (tu < v->t)
					{
						v->t = tu;
						fh.set_pr(v, tu);
					}
				}
				c = c->next;
			}
		}
		//fh.print();
		//...
		//if (n > 17850) fh.print();
		//cout << (++n) << "/" << ht.size() << endl;


	}

	////...
	ofstream myfile;
	myfile.open("C:\\Users\\Yassir\\Downloads\\RoadNetworks\\vis\\points.js");
	myfile << endl << "var plottedPoints = [" << endl;
	E = ht.E;
	for (int N = ht.N; N--; E++)
	{
		for (Chain<struct Entry<Vertex, unsigned int>*> *c = E->first; c != nullptr; c = c->next)
		{
			Vertex *u = c->var->value;
			//cout << u->id << ":" << u->t << endl;
			if (200000 < u->t && u->t < 230000)
			{
				myfile << "\t[" << (float)u->lon / 1000000 << "," <<  (float)u->lat / 1000000  << "]," << endl;
			}
			
		}
	}
	myfile << "];" << endl;
	myfile << endl << "var centralMarker =" << endl;
	myfile << "\t[" << (float)sr->lon / 1000000 << "," << (float)sr->lat / 1000000 << "]" << endl;
	myfile << ";" << endl;
	myfile.close();
}

int RoadNetwork::hashCode(unsigned int n, int N)
{
	return n%N;
}

Vertex *RoadNetwork::select_first()
{
	KeyList<Vertex, unsigned int> *L = ht.E;
	while (!L->n) L++;
	return L->first->var->value;
}