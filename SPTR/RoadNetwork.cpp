#include "stdafx.h"
#include "RoadNetwork.h"

//RoadNetwork::~RoadNetwork()
//{
//}

using namespace std;

Vertex::~Vertex()
{
	Chain<struct Arc> *next;
	while (neighbors != nullptr)
	{
		next = neighbors->next;
		delete neighbors;
		neighbors = next;
	}

	while (predecessors != nullptr)
	{
		next = predecessors->next;
		delete predecessors;
		predecessors = next;
	}
}

void RoadNetwork::readfromfile(const char* file, float latsr, float lonsr)
{
	clock_t ts, te;

	std::cout << "Reading from file " << file << endl;

	size_t length;
	int fd = _open(file, O_RDONLY);
	struct stat sb;
	fstat(fd, &sb);
	length = sb.st_size;

	auto f = static_cast<const char*>(mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0u));
	auto l = f + length;

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
		// We only add the vertexes that are not too far from the source
		// ie not further than the highest interest time (targetTime or targetTimeHigh, depending on 
		// to which question we answer : represented by III) + 1 hour
		// The travel time is estimated with : the segment length between the source and the point divided by 130 km/h
		if (distang((float)lat / 1000000, (float)lon / 1000000, latsr, lonsr) <= 130.*((float)(III ? targetTimeHigh : targetTime) / 3600000. + 1.) / 6371.)
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
}

void RoadNetwork::addVertex(unsigned int id, int lat, int lon)
{
	ht.add(new Vertex(id, lat, lon), id);
	n++;
}

bool RoadNetwork::addArc(unsigned int frid, unsigned int toid, int t)
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

int RoadNetwork::Dijkstra(Vertex *sr)
{
	if (sr == 0)
	{
		std::cout << "Vertex not found!" << endl;
		return 1;
	}

	clock_t ts, te;
	std::cout << "Beginning Dijkstra:" << endl;
	ts = clock();

	//Cleaning from last executiion
	KeyList<struct Vertex, unsigned int> *E = ht.E;
	for (int N = ht.N; N--; E++)
		for (Chain<struct Entry<struct Vertex, unsigned int>*> *c = E->first; c != nullptr; c = c->next)
			c->var->value->computed = false;

	//Processing sr
	this->sr = sr;
	sr->t = 0;
	sr->computed = true;
	sr->IIed = false;

	FibonacciHeap<struct Vertex> fh;

	//Adding first neighbrs
	Chain<Arc> *c = sr->neighbors;
	while (c != nullptr)
	{
		c->var.to->prec = sr;
		fh.add(c->var.to, c->var.t);
		c = c->next;
	}

	//Generating nodeDeg
	fh.generate_nodeDeg(ht.n);

	//Main loop
	while (!fh.isEmpty())
	{
		Vertex *vmin = fh.ext_min();

		/* We make the assumption that there are no neighbour points separated by a time superior to an hour
		(ie that you can not be stuck on a road without being able to change direction for more than 1 hour)
		Therefore, once we are considering points whose reach time from the source is superior to
		(the max interesting time for our algorithm) + the max time between two vertices maxt
		we can exit the loop. */
		if (vmin->t > (III ? targetTimeHigh : targetTime) + maxt) break;

		vmin->computed = true;

		// Si on a d�pass� targetTimeHigh, alors on marque notre anc�tre qui est � targetTime
		if (vmin->t >= targetTimeHigh && vmin->IIIancestor!=nullptr)
		{
			vmin->IIIancestor->IIIed = true;
			vmin->IIIancestor = nullptr;			// On stoppe la descendance : l'anc�tre que nous venons de marquer l'est d�finitivement
		}

		//Processing neighbors
		Chain<Arc> *c = vmin->neighbors;
		while (c != nullptr)
		{
			Vertex *to = c->var.to;
			//if not in the heap
			if (!c->var.to->computed)
			{
				if (to->myFHc == nullptr)
				{
					fh.add(to, vmin->t + c->var.t);
					to->prec = vmin;
				}
				else
				{
					unsigned int tu = vmin->t + c->var.t;
					if (tu < to->t)
					{
						to->prec = vmin;
						fh.set_pr(to, tu);
					}
				}

				// Si on est II-ed (c'est � dire situ� exactement � targetTime), alors on est candidat pour �tre III-ed
				// donc il faut passer � notre descendance un pointeur vers nous
				if (vmin->IIed)	to->IIIancestor = vmin;
				// Si on a un anc�tre potentiellement III-able, on doit le passer � notre descendance
				if (vmin->IIIancestor != nullptr && to->prec == vmin) to->IIIancestor = vmin->IIIancestor;

			}

			if (to->prec == vmin)
			{
				if (vmin->t < targetTime && to->t >= targetTime)
				{
					to->IIed = true;
					to->interLat = interpolation(vmin->lat, to->lat, vmin->t, to->t);
					to->interLon = interpolation(vmin->lon, to->lon, vmin->t, to->t);
				}
				else to->IIed = false;
			}

			c = c->next;
		}
	}

	te = clock();
	std::cout << "Dijkstra ended sucessfully!" << endl;
	std::cout << "Ellapsed time: " << (double)(te - ts) / CLOCKS_PER_SEC * 1000. << "ms" << endl;

	return 0;
}

Chain<struct Vertex*>* RoadNetwork::computeTout(Vertex *sr)
{
	if (sr == 0)
	{
		std::cout << "Vertex not found!" << endl;
		return nullptr;
	}
	Chain<struct Vertex*> *Tout = nullptr;

	clock_t ts, te;
	std::cout << "Beginning Dijkstra:" << endl;
	ts = clock();

	//Cleaning from last execution
	KeyList<struct Vertex, unsigned int> *E = ht.E;
	for (int N = ht.N; N--; E++)
		for (Chain<struct Entry<struct Vertex, unsigned int>*> *c = E->first; c != nullptr; c = c->next)
			c->var->value->computed = false;

	//Processing sr
	this->sr = sr;
	sr->t = 0;
	sr->computed = true;
	sr->IIed = false;

	FibonacciHeap<struct Vertex> fh;

	//Adding first neighbrs
	Chain<Arc> *c = sr->neighbors;
	while (c != nullptr)
	{
		c->var.to->prec = sr;
		fh.add(c->var.to, c->var.t);
		c = c->next;
	}

	//Generating nodeDeg
	fh.generate_nodeDeg(ht.n);

	//Main loop
	while (!fh.isEmpty())
	{
		Vertex *vmin = fh.ext_min();

		/* We make the assumption that there are no neighbour points separated by a time superior to an hour
		(ie that you can not be stuck on a road without being able to change direction for more than 1 hour)
		Therefore, once we are considering points whose reach time from the source is superior to
		(the max interesting time for our algorithm) + maxt
		we can exit the loop. */
		if (vmin->t > (III ? targetTimeHigh : targetTime) + maxt) break;

		vmin->computed = true;
		// Si on a d�pass� targetTimeHigh, alors on marque notre anc�tre qui est � targetTime
		// et on le met dans Tout
		if (vmin->t >= targetTimeHigh && vmin->IIIancestor!=nullptr)
		{
			if (!vmin->IIIancestor->IIIed)
			{
				Tout = new Chain<struct Vertex *>(vmin->IIIancestor, Tout);
			}
			vmin->IIIancestor->IIIed = true;
			vmin->IIIancestor = nullptr;			// On stoppe la descendance : l'anc�tre que nous venons de marquer l'est d�finitivement

		}


		//Processing neighbors
		Chain<Arc> *c = vmin->neighbors;
		while (c != nullptr)
		{
			Vertex *to = c->var.to;
			//if not in the heap
			if (!c->var.to->computed)
			{
				if (to->myFHc == nullptr)
				{
					fh.add(to, vmin->t + c->var.t);
					to->prec = vmin;
				}
				else
				{
					unsigned int tu = vmin->t + c->var.t;
					if (tu < to->t)
					{
						fh.set_pr(to, tu);
						to->prec = vmin;
					}
				}

				// Si on est II-ed (c'est � dire situ� exactement � targetTime), alors on est candidat pour �tre III-ed
				// donc il faut passer � notre descendance un pointeur vers nous
				if (vmin->IIed)	to->IIIancestor = vmin;
				// Si on a un anc�tre potentiellement III-able, on doit le passer � notre descendance
				if (vmin->IIIancestor != nullptr && to->prec == vmin) to->IIIancestor = vmin->IIIancestor;
			}

			if (to->prec == vmin)
			{
				if (vmin->t < targetTime && to->t >= targetTime)
				{
					to->IIed = true;
					to->interLat = interpolation(vmin->lat, to->lat, vmin->t, to->t);
					to->interLon = interpolation(vmin->lon, to->lon, vmin->t, to->t);
				}
				else to->IIed = false;
			}

			c = c->next;
			}
		}

	te = clock();
	std::cout << "Dijkstra ended sucessfully!" << endl;
	std::cout << "Ellapsed time: " << (double)(te - ts) / CLOCKS_PER_SEC * 1000. << "ms" << endl;

	int nbtest = 0;
	Chain<struct Vertex*> *d = Tout;
	while (d != nullptr)
	{
		d = d->next;
		nbtest++;
	}
	std::cout << nbtest << endl;
	return Tout;
	}

Chain<struct Vertex*>* RoadNetwork::computeSin(Vertex *sr)
{
	if (sr == 0)
	{
		std::cout << "Vertex not found!" << endl;
		return nullptr;
	}
	Chain<struct Vertex*> *Sin = nullptr;

	clock_t ts, te;
	std::cout << "Beginning Dijkstra:" << endl;
	ts = clock();

	//Cleaning from last execution
	KeyList<struct Vertex, unsigned int> *E = ht.E;
	for (int N = ht.N; N--; E++)
		for (Chain<struct Entry<struct Vertex, unsigned int>*> *c = E->first; c != nullptr; c = c->next)
		{
			c->var->value->computed = false;
			c->var->value->IIIancestor = nullptr;
			c->var->value->IIed = false;
			c->var->value->IIIed = false;
			c->var->value->prec = nullptr;
			c->var->value->t = 0;
			c->var->value->myFHc = nullptr;
		}


	//Processing sr
	this->sr = sr;
	sr->t = 0;
	sr->computed = true;
	sr->IIed = false;

	FibonacciHeap<struct Vertex> fh;

	//Adding first neighbrs
	Chain<Arc> *c = sr->predecessors;
	while (c != nullptr)
	{
		c->var.to->prec = sr;
		fh.add(c->var.to, c->var.t);
		c = c->next;
	}

	//Generating nodeDeg
	fh.generate_nodeDeg(ht.n);

	//Main loop
	while (!fh.isEmpty())
	{
		Vertex *vmin = fh.ext_min();

		//std::cout << vmin->id << endl;;

		/* We make the assumption that there are no neighbour points separated by a time superior to an hour
		(ie that you can not be stuck on a road without being able to change direction for more than 1 hour)
		Therefore, once we are considering points whose reach time from the source is superior to
		(the max interesting time for our algorithm) + maxt
		we can exit the loop. */
		if (vmin->t > (III ? targetTimeHigh : targetTime) + maxt)	break;

		vmin->computed = true;
		// Si on a d�pass� targetTimeHigh, alors on marque notre anc�tre qui est � targetTime
		// et on le met dans Tout
		if (vmin->t >= targetTimeHigh && vmin->IIIancestor!=nullptr)
		{
			if (!vmin->IIIancestor->IIIed)
			{
				Sin = new Chain<struct Vertex*>(vmin->IIIancestor, Sin);
			}
			vmin->IIIancestor->IIIed = true;
			vmin->IIIancestor = nullptr;			// On stoppe la descendance : l'anc�tre que nous venons de marquer l'est d�finitivement

		}


		//Processing neighbors
		Chain<Arc> *c = vmin->predecessors;
		while (c != nullptr)
		{
			Vertex *to = c->var.to;
			//if not in the heap
			if (!c->var.to->computed)
			{

				if (to->myFHc == nullptr)
				{
					fh.add(to, vmin->t + c->var.t);
					to->prec = vmin;
				}
				else
				{
					unsigned int tu = vmin->t + c->var.t;
					if (tu < to->t)
					{
						fh.set_pr(to, tu);
						to->prec = vmin;
					}
				}

				// Si on est II-ed (c'est � dire situ� exactement � targetTime), alors on est candidat pour �tre III-ed
				// donc il faut passer � notre descendance un pointeur vers nous
				if (vmin->IIed)	to->IIIancestor = vmin;
				// Si on a un anc�tre potentiellement III-able, on doit le passer � notre descendance
				if (vmin->IIIancestor != nullptr && to->prec == vmin) to->IIIancestor = vmin->IIIancestor;
			}

			if (to->prec == vmin)
			{
				if (vmin->t < targetTime && to->t >= targetTime)
				{
					to->IIed = true;
					to->interLat = interpolation(vmin->lat, to->lat, vmin->t, to->t);
					to->interLon = interpolation(vmin->lon, to->lon, vmin->t, to->t);
				}
				else to->IIed = false;
			}
			c = c->next;
		}
	}

	te = clock();
	std::cout << "Dijkstra ended sucessfully!" << endl;
	std::cout << "Ellapsed time: " << (double)(te - ts) / CLOCKS_PER_SEC * 1000. << "ms" << endl;

	int nbtest = 0;
	Chain<struct Vertex*> *d = Sin;
	while (d != nullptr)
	{
		d = d->next;
		nbtest++;
	}
	std::cout << nbtest << endl;
	return Sin;
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
	std::srand((int)time(nullptr));
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

void RoadNetwork::printinfile(const char* file)
{
	int nbrTargetPoints = 0;
	std::cout << "Exporting points to file " << file << endl;
	ofstream myfile;
	myfile.open(file);
	myfile << endl << "var plottedPoints = [" << endl;
	KeyList<struct Vertex, unsigned int> *E = ht.E;
	for (int N = ht.N; N--; E++)
	{
		for (Chain<struct Entry<struct Vertex, unsigned int>*> *c = E->first; c != nullptr; c = c->next)
		{
			Vertex *u = c->var->value;

			if ((u->IIed && !III) || (u->IIIed && III))
			{
				myfile << "\t[" << (float)u->interLat / 1000000 << "," << (float)u->interLon / 1000000 << "]," << endl;
				nbrTargetPoints++;
			}

		}
	}
	myfile << "];" << endl;
	myfile << endl << "var centralMarker =" << endl;
	myfile << "\t[" << (float)sr->lat / 1000000 << "," << (float)sr->lon / 1000000 << "]" << endl;
	myfile << ";" << endl;
	myfile.close();
	std::cout << nbrTargetPoints << " points are matching our expectations" << endl;
}



void RoadNetwork::printroadto(Vertex *to, const char* file)
{
	std::cout << "Exporting points to file " << file << endl;
	ofstream myfile;
	myfile.open(file);
	myfile << endl << "var plottedPoints = [" << endl;
	KeyList<struct Vertex, unsigned int> *E = ht.E;

	while (to != sr)
	{
		myfile << "\t[" << (float)to->lat / 1000000 << "," << (float)to->lon / 1000000 << "]," << endl;
		to = to->prec;
	}
	myfile << "];" << endl;
	myfile << endl << "var centralMarker =" << endl;
	myfile << "\t[" << (float)sr->lat / 1000000 << "," << (float)sr->lon / 1000000 << "]" << endl;
	myfile << ";" << endl;
	myfile.close();
}

float RoadNetwork::distang(float lata, float lona, float lats, float lonb)
{
#define PI 3.141592
	return (float)acos(sin(lata*PI / 180)*sin(lats*PI / 180) + cos(lata*PI / 180)*cos(lats*PI / 180)*cos((lonb - lona)*PI / 180));
}

int RoadNetwork::interpolation(int c1, int c2, int t1, int t2)
{
	float vitesse = (float)(c2 - c1) / (float)(t2 - t1);
	float expectedPoint = c1 + vitesse * (targetTime - t1);
	return (int)expectedPoint;
}