#include "stdafx.h"
#include "RoadNetwork.h"

// Object destructor
RoadNetwork::~RoadNetwork()
{
	// Delete the sorted arrays
	if (SV != nullptr)delete[] SV;
	if (SVinv != nullptr)delete[] SVinv;
}

using namespace std;
// Vertex destructor
Vertex::~Vertex()
{
	// Empty its neighbors list
	if (prec !=nullptr) prec->deleterec();
	if (precinv != nullptr) precinv->deleterec();
	if (neighbors != nullptr) neighbors->deleterec();
	if (predecessors != nullptr) predecessors->deleterec();
}

/*
Reading the input data file.
Using a memory mapped file for faster execution
*/
void RoadNetwork::readfromfile(const char* file)
{
	clock_t ts, te;

	std::cout << "Reading from file " << file << endl;

	size_t length;
	int fd = _open(file, O_RDONLY);
	struct stat sb;
	fstat(fd, &sb);
	length = sb.st_size;

	// File loading
	auto b = static_cast<const char*>(mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0u));
	auto e = b + length;
	auto f = b;

	unsigned int id, p1, p2;
	int lon, lat, t;

	char *nb = new char[10];
	char *cnb;

	ts = clock();

	// Adding every vertex
	while (*(f++) == 'v')
	{
		cnb = nb;
		do { (*(cnb++) = *(++f)); } while (*f != ' ');
		*cnb = '\0';
		id = (unsigned int)stoul(nb, 0);
		cnb = nb;
		do { (*(cnb++) = *(++f)); } while (*f != ' ');
		*cnb = '\0';
		lon = atoi(nb);
		cnb = nb;
		do { (*(cnb++) = *(++f)); } while (*f != '\n');
		*cnb = '\0';
		lat = atoi(nb);
		addVertex(id, lat, lon);
		f++;
	}
	f--;
	te = clock();
	std::cout << n << " vertexes added." << endl;
	std::cout << "Ellapsed time: " << (double)(te - ts) / CLOCKS_PER_SEC * 1000. << "ms" << endl;

	ts = clock();
	// Adding every arc
	while (*(f++) == 'a')
	{
		cnb = nb;
		do { (*(cnb++) = *(++f)); } while (*f != ' ');
		*cnb = '\0';
		p1 = (unsigned int)stoul(nb, 0);
		cnb = nb;
		do { (*(cnb++) = *(++f)); } while (*f != ' ');
		*cnb = '\0';
		p2 = (unsigned int)stoul(nb, 0);
		cnb = nb;
		do { (*(cnb++) = *(++f)); } while (*f != '\n');
		*cnb = '\0';
		t = atoi(nb);
		addArc(p1, p2, t);
		f++;
	}
	te = clock();
	std::cout << m << " arc added." << endl;
	std::cout << "Ellapsed time: " << (double)(te - ts) / CLOCKS_PER_SEC * 1000. << "ms" << endl;
	munmap((void *)b, length);
}

// Add a vertex to our hashtable
void RoadNetwork::addVertex(unsigned int id, int lat, int lon)
{
	ht.add(new Vertex(id, lat, lon), id);
	n++;
}

// Add an arc : add the destination (toid) to the neighbors list of the stating point (frid)
bool RoadNetwork::addArc(unsigned int frid, unsigned int toid, unsigned int t)
{
	Vertex *fr = ht.find(frid);
	if (fr == nullptr) return false;
	Vertex *to = ht.find(toid);
	if (to == nullptr) return false;
	fr->neighbors = new Chain<Arc>({ to, t }, fr->neighbors);
	to->predecessors = new Chain<Arc>({ fr, t }, to->predecessors);
	m++;
	// Update the maximum length of an arc
	if (t > maxt) maxt = t;
	return true;
}

bool RoadNetwork::Dijkstra(Vertex *sr, unsigned int Vertex::* t, Chain<struct Arc>* Vertex::* neighs, Chain<struct Vertex*>* Vertex::* prec, Vertex**& SV, int& ninCC, bool main, unsigned int TT)
{
	if (sr == 0)
	{
		std::cout << "Vertex not found!" << endl;
		return false;
	}

	// Cleaning from last execution :
	if (main)
	{
		if (SV != nullptr)
		{
			delete[] SV;
			ninCC = 0;
		}
		SV = new Vertex*[n];
	}

	clock_t ts, te;
	//std::cout << "Beginning Dijkstra:" << endl;
	ts = clock();

	// Creating an empty Fibonacci heap to store our vertices during Dijkstra execution
	FibonacciHeap<struct Vertex> fh;

	//Processing the starting point sr
	if (main)				// It is the source point
		this->sr = sr;
	sr->*prec = nullptr;
	sr->myFHc = fh.add(sr, 0);
	
	//Generating nodeDeg
	fh.generate_nodeDeg(ht.n);
	
	//Main loop : filling the fibonacci heap and extracting its minimum
	while (!fh.isEmpty())
	{
		// Extracting the minimum of the Fibonacci heap (the closest point that has not yet been computed)
		// It's priority in the fibonacci heap becomes its reaching time from the source
		fh.minnode->v->*t = fh.minnode->p;
		Vertex *vmin = fh.ext_min();
		vmin->myFHc = nullptr;

		// If we are in the main Dijkstra, we fill the sorted vertex array
		if (main) SV[++ninCC] = vmin;
		// Else, we check if we can stop the main loop : in case we have reached a point from Tout
		// or if we have reached the time limit
		else
		{
			if ((vmin->stopon && vmin->*t == vmin->t + sr->tinv))
			{
				clean_Vertices();
				return true;
			}
			if (vmin->*t > TT) break;
		}

		// Once a vertex is extracted from the heep, it is computed for good 
		// (we have found its minimal reaching time from the source)
		vmin->computed = true;

		// Processing neighbors of the currently selected vertex
		Chain<Arc> *c = vmin->*neighs;
		while (c != nullptr)
		{
			Vertex *to = c->var.to;
			// We must consider to only if it has not been computed yet
			if (!c->var.to->computed)
			{
				// If not in the heap is not yet in the Fibonacci heap, we must add it,
				// with the priority value vmin->t + c->var.t
				// Indeed, there is for now no other path leading to to
				if (to->myFHc == nullptr)
				{
					to->myFHc = fh.add(to, vmin->*t + c->var.t);
					if (main)
					{
						if (to->*prec != nullptr) (to->*prec)->deleterec();
						to->*prec = new Chain<struct Vertex*>(vmin);		// The current fastest way to reach to is through vmin
					}
				}
				// If it was already in the Fibonacci heap,
				// we only update its reaching time if necessary
				else
				{
					unsigned int tu = vmin->*t + c->var.t;
					if (tu < to->myFHc->p)
					{
						fh.set_pr(to->myFHc, tu);
						if (main)
						{
							if (to->*prec != nullptr) (to->*prec)->deleterec();
							to->*prec = new Chain<struct Vertex*>(vmin);		// The current fastest way to reach to is through vmin
						}
					}
					else if (tu == to->myFHc->p && main) to->*prec = new Chain<struct Vertex*>(vmin, to->*prec);
				}
			}
			c = c->next;
		}
	}
	te = clock();
	
	//Cleaning from executiion
	clean_Vertices();

	return false;
}

// Computing the Tout and Sin sets :
Chain<struct Vertex*>* RoadNetwork::computeToutSin(unsigned int TT, unsigned int TTH, Vertex **&SV, int& ninCC, unsigned int Vertex::* t, Chain<struct Vertex*>* Vertex::* prec)
{
	// We first run two dichotomy algorithms,
	// in order to find the points to start the backtracking from
	int iTTH, iTTHpTM;
	int i = 0, j = ninCC - 1; int m;
	while (j - i)
	{
		m = (j + i) / 2;
		if (SV[m]->*t == TTH)
		{
			iTTH = m;
			break;
		}
		else if (SV[m]->*t > TTH) j = m;
		else i = m + 1;
	}
	if (i == j) iTTH = i;

	i = 0; j = ninCC - 1;
	while (j - i)
	{
		m = (j + i ) / 2;
		if (SV[m]->*t == TTH + maxt)
		{
			iTTHpTM = m;
			break;
		}
		else if (SV[m]->*t > TTH + maxt) j = m;
		else i = m + 1;
	}
	if (i == j) iTTHpTM = i;

	// We then backtrack each of those points, with a recursive function
	Chain<struct Vertex*> *Vs = nullptr;
	sr->computed = true;
	#pragma omp parallel for
	for (i = iTTH; i <= iTTHpTM; i++)
		ToutSinrec(TT, TTH, SV[i], Vs, t, prec);

	// Cleaning execution
	clean_Vertices();
	return Vs;
}

// The recursive function that backtraks the points at TTH to its ancestor(s) at TT
int RoadNetwork::ToutSinrec(unsigned int& TT, unsigned int& TTH, Vertex* v, Chain<struct Vertex*>*& Vs, unsigned int Vertex::* t, Chain<struct Vertex*>* Vertex::* prec)
{
	if (v->computed) return 0;
	v->computed = true;
	int nb = 0;
	bool fnd = false;
	Chain<struct Vertex*> *c = v->*prec;
	// Considering every ancestor of the current point :
	do
	{
		if (c->var->*t <= TT) fnd = true;
		else nb += ToutSinrec(TT, TTH, c->var, Vs, t, prec);		// Backtracking
	} while ((c = c->next) != nullptr);
	if (fnd) { Vs = new Chain<struct Vertex*>(v, Vs); nb++; }
	return nb;
}


// Two functions to compute Tout and Sin, for defferent targetTime and targetTimeHigh
// -> Returns the points located at targetTime from the source,
//		and that are on a way to a point further than targetTimeHigh
Chain<struct Vertex*>* RoadNetwork::computeTout(unsigned int TT, unsigned int TTH)
{
	return computeToutSin(TT, TTH, SV, ninCC, &Vertex::t, &Vertex::prec);
}
// -> Returns the points from which the source is at exactly targetTime, when 
//		coming from a vertex that is further than targetTimeHigh
Chain<struct Vertex*>* RoadNetwork::computeSin(unsigned int TT, unsigned int TTH)
{
	return computeToutSin(TT, TTH, SVinv, ninCCinv, &Vertex::tinv, &Vertex::precinv);
}

// Computes an approximated reach of the vertex V
unsigned int RoadNetwork::Reach(Vertex* v)
{
	if (v->neighbors == nullptr || v->predecessors == nullptr || (v->neighbors->next == nullptr && v->predecessors->next == nullptr && v->neighbors->var.to == v->predecessors->var.to)) return 0;
	
	clock_t ts, te;
	ts = clock();

	// Must first run 2 Dijkstras with the vertex as a source,
	// with normal and reverted edges, in order to 
	// later compute Tout and Sin
	Dijkstra(v, &Vertex::t, &Vertex::neighbors, &Vertex::prec, SV, ninCC, true, 0);
	Dijkstra(v, &Vertex::tinv, &Vertex::predecessors, &Vertex::precinv, SVinv, ninCCinv, true, 0);
	
	// Time limits
	unsigned int tmin = 0, tmax = 8*3600*1000;
	Chain<struct Vertex*> *cS;
	Chain<struct Vertex*> *cT;

	// The starting bounds for the 2-approx algo
	double TT = 7.5*60*1000/2;
	double TTH = sqrt(2)*7.5*60.*1000./2.;
	double f = 0;
	// Main loop
	while (true)
	{
		// Computing Tout and Sin
		Chain<struct Vertex*> *Tout = computeTout(ceil(TT), ceil(TTH));
		Chain<struct Vertex*> *Sin = computeSin(ceil(TT), ceil(TTH));

		// Looking for a shortest path from a point from Sin to a point in Tout
		// that goes through the source vertex v
		for (cT = Tout; cT != nullptr; cT = cT->next)
			cT->var->stopon = true;
		for (cS = Sin; cS != nullptr; cS = cS->next)
			if (Dijkstra(cS->var, &Vertex::t2, &Vertex::neighbors, nullptr, SV, ninCC, false, ceil(2. * TT) + 2 * maxt)) break;
		for (cT = Tout; cT != nullptr; cT = cT->next)
			cT->var->stopon = false;

		// Cleaning from execugtion
		if (Tout != nullptr) Tout->deleterec();
		if (Sin != nullptr) Sin->deleterec();

		// Updating the bounds for the next execution, depending on the result of the above search
		if (!f) f = (cS != nullptr) ? sqrt(2) : 1/sqrt(2);
		if (f>=1 == (cS != nullptr))
		{
			TT *= f;
			TTH *= f;
			std::cout << "TT=" << TT << " TTH=" << TTH << endl;
			if (TTH < 1) break;
		}
		else
		{
			if (f >=1) TTH *= sqrt(2);
			else TT /= sqrt(2);
			break;
		}
	}
	te = clock();
	std::cout << "Reach ended! " << TT << "<=r<=" << TTH << std::endl;
	std::cout << "Ellapsed time: " << (double)(te - ts) / CLOCKS_PER_SEC * 1000. << "ms" << endl;
	return (unsigned int)TTH;
}

// Exactly the same as Reach, except that when we know that the reach is under
// 7,5 minutes, we stop the computation
unsigned int RoadNetwork::ReachApprox(Vertex* v)
{
	if (v->neighbors == nullptr || v->predecessors == nullptr || (v->neighbors->next == nullptr && v->predecessors->next == nullptr && v->neighbors->var.to == v->predecessors->var.to)) return 0;

	clock_t ts, te;
	ts = clock();

	Dijkstra(v, &Vertex::t, &Vertex::neighbors, &Vertex::prec, SV, ninCC, true, 0);
	Dijkstra(v, &Vertex::tinv, &Vertex::predecessors, &Vertex::precinv, SVinv, ninCCinv, true, 0);

	unsigned int tmin = 0, tmax = 8 * 3600 * 1000;
	Chain<struct Vertex*> *cS;
	Chain<struct Vertex*> *cT;

	double TT = 7.5 * 60 * 1000 / 2;
	double TTH = sqrt(2)*7.5*60.*1000. / 2.;
	double f = 0;
	while (true)
	{
		Chain<struct Vertex*> *Tout = computeTout(ceil(TT), ceil(TTH));
		Chain<struct Vertex*> *Sin = computeSin(ceil(TT), ceil(TTH));

		for (cT = Tout; cT != nullptr; cT = cT->next)
			cT->var->stopon = true;
		for (cS = Sin; cS != nullptr; cS = cS->next)
			if (Dijkstra(cS->var, &Vertex::t2, &Vertex::neighbors, nullptr, SV, ninCC, false, ceil(2. * TT) + 2 * maxt)) break;
		for (cT = Tout; cT != nullptr; cT = cT->next)
			cT->var->stopon = false;

		if (Tout != nullptr) Tout->deleterec();
		if (Sin != nullptr) Sin->deleterec();

		if (f==0)
		{
			if (cS != nullptr) f = sqrt(2);
			else return (unsigned int)TTH;
		}
		if (cS != nullptr)
		{
			TT *= f;
			TTH *= f;
			std::cout << "TT=" << TT << " TTH=" << TTH << endl;
			if (TTH == 0) break;
		}
		else
		{
			TTH *= sqrt(2);
			break;
		}
	}
	te = clock();
	std::cout << "Reach ended! " << TT << "<=r<=" << TTH << std::endl;
	std::cout << "Ellapsed time: " << (double)(te - ts) / CLOCKS_PER_SEC * 1000. << "ms" << endl;
	return (unsigned int)TTH;
}

// Prints a vertex's ID and its estimated reach
// in the output file
void RoadNetwork::printReach(const char* file, int nb)
{
	int nbV = nb;
	ofstream myfile;
	
	// Opening the file
	myfile.open(file, std::ios_base::app);
	if (!myfile.is_open()) return;
	do
	{
		Vertex *v = select_vertex_rand();
		int A = ReachApprox(v);
		myfile << v->id << ";" << A << endl;		// Writing
	} while (--nb);

	myfile.close();
}

// hashCode function : id of the vertex % length of the hashtable
int RoadNetwork::hashCode(unsigned int n, int N)
{
	return n%N;
}

// Returns the first vertex in the hashtable
Vertex *RoadNetwork::select_first_vertex()
{
	KeyList<struct Vertex, unsigned int> *L = ht.E;
	while (!L->n) L++;
	return L->first->var->value;
}

// Returns a random vertex in the hashtable (or rather the first vertex of a
// not-empty tray in the hastable)
Vertex *RoadNetwork::select_vertex_rand()
{
	if (ht.n == 0) return nullptr;
	int r = std::rand() % ht.N;
	KeyList<struct Vertex, unsigned int> *L = ht.E + r;
	while (!L->n)
	{
		if (++L == ht.E + ht.N) L = ht.E;
	}
	return L->first->var->value;
}

// Returns the vertex given a specified Id
Vertex *RoadNetwork::select_vertex_id(int id)
{
	return ht.find(id);
}

// Returns the vertex given its coordinates (not really safe if there is no corresponding vertex)
Vertex *RoadNetwork::select_vertex_coords(int lat, int lon)
{
	KeyList<Vertex, unsigned int> *E = ht.E;
	for (int N = ht.N; N--; E++)
	{
		for (Chain<struct Entry<Vertex, unsigned int>*> *c = E->first; c != nullptr; c = c->next)
		{
			Vertex *u = c->var->value;
			if (u->lat == lat && u->lon == lon)
				return u;
		}
	}
	return nullptr;
}

// Returns the nearest vertex of a set of coordinates (safer than select_vertex_coords)
Vertex *RoadNetwork::select_vertex_nearest(float lat, float lon)
{
	KeyList<Vertex, unsigned int> *E = ht.E;
	double damin = 1;
	Vertex *umin = select_first_vertex();
	for (int N = ht.N; N--; E++)
	{
		for (Chain<struct Entry<Vertex, unsigned int>*> *c = E->first; c != nullptr; c = c->next)
		{
			Vertex *u = c->var->value;
			// Finding the minimum distance
			if (distang((float)u->lat / 1000000, (float)u->lon / 1000000, lat, lon) < damin)
			{
				damin = distang((float)u->lat/1000000, (float)u->lon / 1000000, lat, lon);
				umin = u;
			}
		}
	}
	return umin;
}

// Angular distance
float RoadNetwork::distang(float lata, float lona, float lats, float lonb)
{
#define PI 3.141592
	return (float)acos(sin(lata*PI / 180)*sin(lats*PI / 180) + cos(lata*PI / 180)*cos(lats*PI / 180)*cos((lonb - lona)*PI / 180));
}

// To clean between 2 Dijkstra's, we must put the vertices computed value on false
void RoadNetwork::clean_Vertices()
{
	KeyList<struct Vertex, unsigned int> *E = ht.E;
	for (int N = ht.N; N--; E++)
		for (Chain<struct Entry<struct Vertex, unsigned int>*> *c = E->first; c != nullptr; c = c->next)
			c->var->value->computed = false;	
}