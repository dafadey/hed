/*
 * this is example implementation for 1d EMP resonatior spectra
 * 
 * 1. ELECTRODYNAMICS
 * 
 * equations are:
 * dE/dt = dB/dx
 * dB/dt = dE/dx
 * 
 * 2. SOME LINEAR ALGEBRA
 * 
 * in terms of linear algebra it reads as follows
 * 
 * dV/dt = L V            (1)
 * 
 * where
 * 
 *     / E \        / 0   d/dx \
 * V = |   |    L = |          |
 *     \ B /        \ d/dx   0 /
 * 
 * with zero boundary conditions for E it has first eigenvalue
 * l0 = i * pi / L
 * and eigenvector
 *      /   sin pi/L * x  \
 * V0 = |                 |
 *      \ -i cos pi/L * x /
 * 
 * with complex time ie
 * dV/dt = i L V            (2)
 * new L` = i L will have same eigenvector but real negative eigen value
 * l0 = - pi / L
 * 
 * 3.COMPLEX TIME METHOD
 * 
 * which allows to run iterative diagonalization procedure by simple
 * advancing (2) with euller's scheme or so.
 * being normalized at each step first eigenvectors expected to survive
 * among others in solution which is apparantly a linear combination of
 * all eigenvectors.
 * finding next eigenvector is of the question since it is not guaranteed
 * to have another parity so it could be hard to withdraw first eigenvalue
 * from solution each time. Also eigenvectors may be non-orthogonal.
 */

#include <iostream>
#include <sstream>
#include <array>
#include <cmath>
#include <set>
#include <limits>
#include <assert.h>
#include <algorithm>
#include "resonators.h"

int main(int argc, char* argv[])
{
  /*
  resonator_base* SR= new structured_resonator(10,10,10.,10.);
  resonator_base* UR = new unstructured_resonator("70x101resonator.svg",10);
  SR->withdraw(UR);
  */
  
  if(argc==2 && std::string(argv[1]) == std::string("-h"))
  {
    std::cout << "./find_2d_eigen mode=[UNSTRUCTURED/STRUCTURED/ANALYTIC, UNSTRUCTURED is default] [options]\n";
    std::cout << "for unstructured opts are: N=<number_of_steps> geo=\"<filename>\" seed=<edge seeding value> dtf=<dt factor - i.e. dt = dtf * min_edge>\n";
    std::cout << "for   structured opts are: N=<number_of_steps> lx=<size of resonator along x> ly=<size of resonator along y> nx=<number of divisions along x> ny=<number of divisions along y> dt = dtf * min_edge>\n";
    std::cout << "for     analytic opts are: lx=<size of resonator along x> ly=<size of resonator along y>\n";
    std::cout << "if you do not want all modes to be found set allmodes=no, default is allmodes=yes, NOTE: ANALYTIC run will find only half of electromagneitc modes with same sign of eigenvalue\n";
    std::cout << "EXAMPLES:\n" << "\t./find_2d_eigen mode=UNSTRUCTURED allmodes=no N=3000 geo=70x101resonator.svg dtf=0.1 seed=13.0 output=ulambdas.dat\n" <<
    "\t./find_2d_eigen mode=STRUCTURED allmodes=no N=3000 lx=101 ly=70 nx=11 ny=8 dtf=0.1 output=slambdas.dat\n" <<
    "\t./find_2d_eigen mode=ANALYTIC lx=101 ly=70 nx=11 ny=8 output=alambdas.dat\n";
    return 0;
  }

  enum erun_type {UNSTRUCTURED,
                  STRUCTURED,
                  ANALYTIC};
  
  
  std::string filename="70x101resonator.svg";
  std::string lfilename="lambdas.dat";
  int n;
  double lx=101.;
  double ly=70.;
  int nx=11;
  int ny=8;
  double seed=13.;
  double dtf=0.05;
  std::string all_modes="yes";
  
  erun_type run_type = UNSTRUCTURED;
  
  std::string mode;
  
  #define PARSE(ARG) if(name == #ARG) { sval >> ARG; continue;}
	#define PARSE2(ARG, VAR) if(name == #ARG) { sval >> VAR; continue;}
	for(int i=1;i<argc;i++)
	{
		std::string inp = std::string(argv[i]);
		size_t pos = inp.find("=");
		if(pos == std::string::npos)
			printf("you specified parameter wrong way use <name>=<value> format. NOTE: no \'-\' and spaces\n");
		else
		{
			std::string name = inp.substr(0,pos);
			std::stringstream sval;
			sval << inp.substr(pos+1,std::string::npos);
			printf("parameter[%d] has name %s and value %s\n",i-1,name.c_str(), sval.str().c_str());
			PARSE2(geo, filename);
			PARSE2(output, lfilename);
			PARSE2(N,n);
			PARSE(dtf);
			PARSE(lx);
			PARSE(ly);
			PARSE(nx);
			PARSE(ny);
      PARSE(mode);
      PARSE2(allmodes, all_modes);
    }
  }
  if(mode=="UNSTRUCTURED")
    run_type = UNSTRUCTURED;
  else if(mode=="STRUCTURED")
    run_type = STRUCTURED;
  else if(mode=="ANALYTIC")
    run_type = ANALYTIC;
  else
  {
    std::cerr << "ERROR: unknown mode\n";
    return -1;
  }

  if(run_type == ANALYTIC)
  {
    std::vector<double> kx;
    std::vector<double> ky;

    for(int i=0; i!=nx; i++)
      kx.push_back((double) i * M_PI / lx);

    for(int i=0; i!=ny; i++)
      ky.push_back((double) i * M_PI / ly);

    std::vector<double> w;
    for(int i(0); i != nx; i++)
    {
      for(int j(0); j != ny; j++)
        w.push_back(sqrt(kx[i] * kx[i] + ky[j] * ky[j]));
    }

    std::sort(w.begin(), w.end(), [](const double& a, const double& b){return a > b;});
    std::cout << w.size() << '\n';
    for(auto _w : w)
      std::cout << -_w << '\n';

    std::ofstream lf(lfilename.c_str());
    lf << w.size() << '\n';
    for(auto _w : w)
      lf << -_w << '\n';
    lf.close();
    return 0;
  }

  system((std::string("mkdir -p ") + lfilename + std::string(".files")).c_str());

  resonator_base* r;
  
  double min_edge(std::numeric_limits<double>::max());

  int num_modes(0);

  if(run_type == UNSTRUCTURED)
  {
    r = new unstructured_resonator(filename, seed);
    unstructured_resonator* ur = (unstructured_resonator*)r;
    
    for(auto e_ptr : ur->m.edges)
    {
      double x = e_ptr->p1->x - e_ptr->p2->x;
      double y = e_ptr->p1->y - e_ptr->p2->y;
      min_edge = std::min(min_edge, sqrt(x * x + y * y));
    }
    num_modes = ur->B.size() + ur->E.size() - ur->bndry.size();
  }
  else if(run_type == STRUCTURED)
  {
    r = new structured_resonator(nx, ny, lx, ly);
    structured_resonator* sr = (structured_resonator*)r;

    min_edge = std::min(sr->lx/(double) sr->nx, sr->ly/(double) sr->ny);
    
    num_modes = sr->B.size() + sr->E.size() - (sr->nx + sr->ny) * 2;
  }
  std::cout << "all modes is " << all_modes << '\n';
  if(all_modes == std::string("no")) // only electromagnetic modes with same sign of eigen value will be found
    num_modes = r->B.size();
  
  cplx dt;
  if(run_type == UNSTRUCTURED)
    dt = cplx(.0, dtf * min_edge);
  else if(run_type == STRUCTURED)
    dt = cplx(.0, dtf * min_edge);
  
  std::cerr << "doing " << n << " steps\n";
  std::cerr << "with dt=" << dt << '\n';
  double l(.0);
  
  
  std::cout  << "filling in random values for e\n";
  for(auto& b : r->B)
    b = cplx(.0, 1.0+(double) rand() / (double) RAND_MAX);

  std::cout << "\tdone\n";
  
  
  std::cout << "preparing resonators\n";
  
  
  std::vector<resonator_base*> rs;
  
  for(int i = 0; i != num_modes; i++)
  {
    if(run_type == UNSTRUCTURED)
    {
      unstructured_resonator* ur=(unstructured_resonator*)r;
      unstructured_resonator* r_new = new unstructured_resonator();
      *r_new = *ur;
      r_new->bndry = ur->bndry;
      r_new->E = ur->E;
      r_new->B = ur->B;
      rs.push_back(r_new);
    }
    else if(run_type == STRUCTURED)
    {
      structured_resonator* sr=(structured_resonator*)r;
      structured_resonator* r_new = new structured_resonator();
      *r_new = *sr;
      r_new->E = sr->E;
      r_new->B = sr->B;
      rs.push_back(r_new);
    }
  }
  std::cout << "\tdone\n";

  std::vector<double> lambdas(rs.size());
  

//#define SINGLE_THREADED

#ifndef SINGLE_THREADED
  for(int i = 0; i != 10; i++)
  {
    for(int j = 0; j < rs.size(); j++)
    {
      for(int q = 0; q < j; q++)
        rs[j]->withdraw(rs[q]);
      lambdas[j] = rs[j]->step(dt);
    }
  }
#endif

  for(int i = 0; i != n; i++)
  {
#ifdef SINGLE_THREADED
    for(int j = 0; j < rs.size(); j++)
    {
      for(int q = 0; q < j; q++)
        rs[j]->withdraw(rs[q]);
      lambdas[j] = rs[j]->step(dt);
    }
#else
    #pragma omp parallel for
    for(int j = 0; j < rs.size(); j++)
      lambdas[j] = rs[j]->step(dt);

    #pragma omp parallel for
    for(int j = 0; j < rs.size(); j++)
    {
      for(int q = 0; q < j; q++)
        rs[j]->withdraw(rs[q]);
    }
#endif

    std::cout << i << '/' << n << '\n';
  }
  
  //dump eigen values
  std::ofstream lf(lfilename.c_str());
  lf << lambdas.size() << '\n';
  for(int j = 0; j < rs.size(); j++)
    lf << lambdas[j] << '\n';
  lf.close();
  
  for(const auto& _l : lambdas)
    std::cout << _l << '\n';
 

  //dump eigen vectors
  if(run_type == UNSTRUCTURED)
  {
    for(size_t rsi(0); rsi != rs.size(); rsi++)
    {
      unstructured_resonator* ur = (unstructured_resonator*)rs[rsi];
      std::ofstream of((lfilename+".files/fields"+std::to_string(rsi)+".debug").c_str());//, std::ios_base::app);
      for(size_t e_i(0); e_i != ur->m.edges.size(); e_i++)
        of << *ur->m.edges[e_i] << ' ' << ur->E[e_i].real() << '\n';
      for(size_t t_i(0); t_i != ur->m.triangles.size(); t_i++)
        of << *ur->m.triangles[t_i] << ' ' << ur->B[t_i].imag() << '\n';
      of.close();
    }
  }
  else if(run_type == STRUCTURED)
  {
    for(size_t rsi(0); rsi != rs.size(); rsi++)
    {
      structured_resonator* sr = (structured_resonator*)rs[rsi];
      std::ofstream of((lfilename+".files/fields"+std::to_string(rsi)+".debug").c_str());//, std::ios_base::app);
      double dx = sr->lx/(double) sr->nx;
      double dy = sr->ly/(double) sr->ny;
      for(size_t e_i(0); e_i != sr->E.size(); e_i++)
      {
        if(e_i < (sr->nx+1)*sr->ny)
        {
          int i=e_i;
          int ix = i % (sr->nx+1);
          int iy = i / (sr->nx+1);
          of << "e " << dx*(double)ix << ' ' << dy*(double)iy << ' ' << dx*(double)ix << ' ' << dy*(double)iy+dy << ' ' << sr->E[e_i].real() << '\n';
        }
        else
        {
          int i = e_i - (sr->nx+1)*sr->ny;
          int ix = i % sr->nx;
          int iy = i / sr->nx;
          of << "e " << dx*(double)ix << ' ' << dy*(double)iy << ' ' << dx*(double)ix+dx << ' ' << dy*(double)iy << ' ' << sr->E[e_i].real() << '\n';
        }
      }
      for(size_t t_i(0); t_i != sr->B.size(); t_i++)
      {
        int ix = t_i % sr->nx;
        int iy = t_i / sr->nx;
        of << "box " << (double)ix*dx << ' ' << (double)iy*dy << ' '
                   << (double)ix*dx+dx << ' ' << (double)iy*dy+dy << ' '
                   << sr->B[t_i].imag() << '\n';
      }
      of.close();
    }
  }
  return 0;
}
