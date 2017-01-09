#include "stdafx.h"
#include "RoadNetwork.h"

RoadNetwork::~RoadNetwork()
{
	if (SV != nullptr)delete[] SV;
	if (SVinv != nullptr)delete[] SVinv;
}

using namespace std;

Vertex::~Vertex()
{
	if (prec !=nullptr) prec->deleterec();
	if (precinv != nullptr) precinv->deleterec();
	if (neighbors != nullptr) neighbors->deleterec();
	if (predecessors != nullptr) predecessors->deleterec();
}

void RoadNetwork::readfromfile(const char* file)
{
	clock_t ts, te;

	std::cout << "Reading from file " << file << endl;

	size_t length;
	int fd = _open(file, O_RDONLY);
	struct stat sb;
	fstat(fd, &sb);
	length = sb.st_size;

	auto b = static_cast<const char*>(mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0u));
	auto e = b + length;
	auto f = b;

	unsigned int id, p1, p2;
	int lon, lat, t;

	char *nb = new char[10];
	char *cnb;

	ts = clock();

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

void RoadNetwork::addVertex(unsigned int id, int lat, int lon)
{
	ht.add(new Vertex(id, lat, lon), id);
	n++;
}

bool RoadNetwork::addArc(unsigned int frid, unsigned int toid, unsigned int t)
{
	Vertex *fr = ht.find(frid);
	if (fr == nullptr) return false;
	Vertex *to = ht.find(toid);
	if (to == nullptr) return false;
	fr->neighbors = new Chain<Arc>({ to, t }, fr->neighbors);
	to->predecessors = new Chain<Arc>({ fr, t }, to->predecessors);
	m++;
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

	//Processing sr
	sr->computed = true;
	sr->*t = 0;
	if (main)
	{
		this->sr = sr;
		SV[0] = sr;
	}

	FibonacciHeap<struct Vertex> fh;

	//Adding first neighbrs
	Chain<Arc> *c = sr->*neighs;
	while (c != nullptr)
	{
		if (main)
		{
			if (c->var.to->*prec != nullptr)(c->var.to->*prec)->deleterec();
			c->var.to->*prec = new Chain<struct Vertex*>(sr);
		}
		c->var.to->myFHc = fh.add(c->var.to, c->var.t);
		c = c->next;
	}

	//Generating nodeDeg
	fh.generate_nodeDeg(ht.n);
	
	//Main loop
	while (!fh.isEmpty())
	{
		fh.minnode->v->*t = fh.minnode->p;

		Vertex *vmin = fh.ext_min();
		vmin->myFHc = nullptr;

		if (main) SV[++ninCC] = vmin;
		else
		{
			if ((vmin->stopon && vmin->*t == vmin->t + sr->tinv))
			{
				clean_Vertices();
				return true;
			}
			if (vmin->*t > TT) break;
		}

		vmin->computed = true;

		//Processing neighbors
		Chain<Arc> *c = vmin->*neighs;
		while (c != nullptr)
		{
			Vertex *to = c->var.to;
			//if not in the heap
			if (!c->var.to->computed)
			{
				if (to->myFHc == nullptr)
				{
					to->myFHc = fh.add(to, vmin->*t + c->var.t);
					if (main)
					{
						if (to->*prec != nullptr) (to->*prec)->deleterec();
						to->*prec = new Chain<struct Vertex*>(vmin);
					}
				}
				else
				{
					unsigned int tu = vmin->*t + c->var.t;
					if (tu < to->myFHc->p)
					{
						fh.set_pr(to->myFHc, tu);
						if (main)
						{
							if (to->*prec != nullptr) (to->*prec)->deleterec();
							to->*prec = new Chain<struct Vertex*>(vmin);
						}
					}
					else if (tu == to->myFHc->p && main) to->*prec = new Chain<struct Vertex*>(vmin, to->*prec);
				}
			}
			c = c->next;
		}
	}
	te = clock();
	//std::cout << "Dijkstra ended sucessfully!" << endl;
	//std::cout << "Ellapsed time: " << (double)(te - ts) / CLOCKS_PER_SEC * 1000. << "ms" << endl;

	//Cleaning from executiion
	clean_Vertices();

	return false;
}


Chain<struct Vertex*>* RoadNetwork::computeToutSin(unsigned int TT, unsigned int TTH, Vertex **&SV, int& ninCC, unsigned int Vertex::* t, Chain<struct Vertex*>* Vertex::* prec)
{
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

	Chain<struct Vertex*> *Vs = nullptr;
	sr->computed = true;
	#pragma omp parallel for
	for (i = iTTH; i <= iTTHpTM; i++)
		ToutSinrec(TT, TTH, SV[i], Vs, t, prec);

	clean_Vertices();
	return Vs;
}

int RoadNetwork::ToutSinrec(unsigned int& TT, unsigned int& TTH, Vertex* v, Chain<struct Vertex*>*& Vs, unsigned int Vertex::* t, Chain<struct Vertex*>* Vertex::* prec)
{
	if (v->computed) return 0;
	v->computed = true;
	int nb = 0;
	bool fnd = false;
	Chain<struct Vertex*> *c = v->*prec;
	do
	{
		if (c->var->*t <= TT) fnd = true;
		else nb += ToutSinrec(TT, TTH, c->var, Vs, t, prec);
	} while ((c = c->next) != nullptr);
	if (fnd) { Vs = new Chain<struct Vertex*>(v, Vs); nb++; }
	return nb;
	/*while (v->*prec->*t >= TT)
	{
		v = v->*prec;
		if (v->computed) break;
		//v->computed = true;
	}
	if (!v->computed && v->*prec->*t < TT)
	{
		v->computed = true;
		Tout = new Chain<struct Vertex*>(v, Tout);
	}*/
}

Chain<struct Vertex*>* RoadNetwork::computeTout(unsigned int TT, unsigned int TTH)
{
	return computeToutSin(TT, TTH, SV, ninCC, &Vertex::t, &Vertex::prec);
}
Chain<struct Vertex*>* RoadNetwork::computeSin(unsigned int TT, unsigned int TTH)
{
	return computeToutSin(TT, TTH, SVinv, ninCCinv, &Vertex::tinv, &Vertex::precinv);
}


unsigned int RoadNetwork::Reach(Vertex* v)
{
	if (v->neighbors == nullptr || v->predecessors == nullptr || (v->neighbors->next == nullptr && v->predecessors->next == nullptr && v->neighbors->var.to == v->predecessors->var.to)) return 0;
	
	clock_t ts, te;
	ts = clock();

	Dijkstra(v, &Vertex::t, &Vertex::neighbors, &Vertex::prec, SV, ninCC, true, 0);
	Dijkstra(v, &Vertex::tinv, &Vertex::predecessors, &Vertex::precinv, SVinv, ninCCinv, true, 0);
	
	unsigned int tmin = 0, tmax = 8*3600*1000;
	Chain<struct Vertex*> *cS;
	Chain<struct Vertex*> *cT;

	double TT = 7.5*60*1000/2;
	double TTH = sqrt(2)*7.5*60.*1000./2.;
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

string currenttime();
string currenttime()
{
	time_t t = time(nullptr);
	struct tm * now = localtime(&t);
	string ct;
	char date[20];
	snprintf(date, sizeof(date), "%02d/%02d/%04d %02d:%02d:%02d", now->tm_mday, now->tm_mon + 1, now->tm_year + 1900, now->tm_hour, now->tm_min, now->tm_sec);
	return string(date);
}

void RoadNetwork::printReach(const char* file, int nb)
{
	int nbV = nb;
	ofstream myfile;
	
	myfile.open(file, std::ios_base::app);
	if (!myfile.is_open()) return;
	//myfile << "Beguinning at " << currenttime()	<< endl << endl;
	do
	{
		Vertex *v = select_vertex_rand();
		int A = ReachApprox(v);
		myfile << v->id << ";" << A << endl;
	} while (--nb);

	//myfile << endl << "Ended at " << currenttime() << endl;
	//myfile << endl << nbV << " vertices computed" << endl;
	myfile.close();
}




int RoadNetwork::hashCode(unsigned int n, int N)
{
	return n%N;
}

Vertex *RoadNetwork::select_first_vertex()
{
	KeyList<struct Vertex, unsigned int> *L = ht.E;
	while (!L->n) L++;
	return L->first->var->value;
}

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

Vertex *RoadNetwork::select_vertex_id(int id)
{
	return ht.find(id);
}

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
			if (distang((float)u->lat / 1000000, (float)u->lon / 1000000, lat, lon) < damin)
			{
				damin = distang((float)u->lat/1000000, (float)u->lon / 1000000, lat, lon);
				umin = u;
			}
		}
	}
	return umin;
}

//void RoadNetwork::printinfile(const char* file)
//{
//	int nbrTargetPoints = 0;
//	std::cout << "Exporting points to file " << file << endl;
//	ofstream myfile;
//	myfile.open(file);
//	myfile << endl << "var plottedPoints = [" << endl;
//	KeyList<struct Vertex, unsigned int> *E = ht.E;
//	for (int N = ht.N; N--; E++)
//	{
//		for (Chain<struct Entry<struct Vertex, unsigned int>*> *c = E->first; c != nullptr; c = c->next)
//		{
//			Vertex *u = c->var->value;
//
//			if ((u->IIed && !III) || (u->IIIed && III))
//			{
//				myfile << "\t[" << (float)u->lat / 1000000 << "," << (float)u->lon / 1000000 << "]," << endl;
//				nbrTargetPoints++;
//			}
//
//		}
//	}
//	myfile << "];" << endl;
//	myfile << endl << "var centralMarker =" << endl;
//	myfile << "\t[" << (float)sr->lat / 1000000 << "," << (float)sr->lon / 1000000 << "]" << endl;
//	myfile << ";" << endl;
//	myfile.close();
//	std::cout << nbrTargetPoints << " points are matching our expectations" << endl;
//}



//void RoadNetwork::printroadto(Vertex *to, const char* file)
//{
//	std::cout << "Exporting points to file " << file << endl;
//	ofstream myfile;
//	myfile.open(file);
//	myfile << endl << "var plottedPoints = [" << endl;
//	KeyList<struct Vertex, unsigned int> *E = ht.E;
//
//	while (to != sr)
//	{
//		myfile << "\t[" << (float)to->lat / 1000000 << "," << (float)to->lon / 1000000 << "]," << endl;
//		to = to->prec;
//	}
//	myfile << "];" << endl;
//	myfile << endl << "var centralMarker =" << endl;
//	myfile << "\t[" << (float)sr->lat / 1000000 << "," << (float)sr->lon / 1000000 << "]" << endl;
//	myfile << ";" << endl;
//	myfile.close();
//}



float RoadNetwork::distang(float lata, float lona, float lats, float lonb)
{
#define PI 3.141592
	return (float)acos(sin(lata*PI / 180)*sin(lats*PI / 180) + cos(lata*PI / 180)*cos(lats*PI / 180)*cos((lonb - lona)*PI / 180));
}
//
//int RoadNetwork::interpolation(int c1, int c2, int t1, int t2)
//{
//	float vitesse = (float)(c2 - c1) / (float)(t2 - t1);
//	float expectedPoint = c1 + vitesse * (targetTime - t1);
//	return (int)expectedPoint;
//}

void RoadNetwork::clean_Vertices()
{
	KeyList<struct Vertex, unsigned int> *E = ht.E;
	for (int N = ht.N; N--; E++)
		for (Chain<struct Entry<struct Vertex, unsigned int>*> *c = E->first; c != nullptr; c = c->next)
			c->var->value->computed = false;	
}