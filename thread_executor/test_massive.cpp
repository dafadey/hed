#include "executor.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <chrono>

struct tc
{
  tc(int Nx, int Ny, double Lx, double Ly) : T(new double[Nx*Ny]),
                                             dx(Lx/(double)Nx),
                                             dy(Ly/(double)Ny),
                                             t(.0),
                                             dt(std::min(0.25*dx*dx, 0.25*dy*dy)),
                                             nx(Nx), ny(Ny)
  {
    for(int i(0); i!=Nx*Ny; i++)
      T[i] = .0;
  }
  ~tc()
  {
    delete[] T;
  }
  double* T;
  double dx;
  double dy;
  double t;
  double dt;
  int nx;
  int ny;
  void dump(const char* filename)
  {
    std::ofstream of(filename);
    of << nx << "\n" << ny << "\n";
    for(int i(0); i != nx * ny; i++)
      of << T[i] << "\n";
    of.close();
  }
};

void step1(void* _a)
{
  tc* a = (tc*) _a;
  const int nx = a->nx;
  const int ny = a->ny;
  double* T = a->T;
  const double dx = a->dx;
  const double dy = a->dy;
  const double dt = a->dt;
  
  for(int yi=1; yi!=ny-1; yi++)
  {
    for(int xi=1; xi!=nx-1; xi++)
    {
      T[yi * nx + xi] += ((T[yi * nx + xi + 1] + T[yi * nx + xi - 1] - 2.0 * T[yi * nx + xi]) / (dx * dx) +
                          (T[(yi + 1) * nx + xi] + T[(yi - 1) * nx + xi] - 2.0 * T[yi * nx + xi]) / (dy * dy)) * dt;
      a->t += dt;
    }
  }
}

void step2(void* _a)
{
  tc* a = (tc*) _a;
  const int nx = a->nx;
  const int ny = a->ny;
  double* T = a->T;
  const double dx = a->dx;
  const double dy = a->dy;
  const double dt = a->dt;
  
  for(int yi=1; yi!=ny-1; yi++)
  {
    for(int xi=1; xi!=nx-1; xi++)
    {
      T[yi * nx + xi] += ((T[yi * nx + xi + 1] + T[yi * nx + xi - 1] - 2.0 * T[yi * nx + xi]) / (dx * dx) +
                          (T[(yi + 1) * nx + xi] + T[(yi - 1) * nx + xi] - 2.0 * T[yi * nx + xi]) / (dy * dy)) * 0.5 * dt;
      a->t += dt;
    }
  }
}

double gauss(double x, double y, double xc, double yc, double wx, double wy)
{
  return exp(-(x-xc)*(x-xc)/wx/wx-(y-yc)*(y-yc)/wy/wy);
}

void set(tc* tc1, double xc, double yc, double wx, double wy)
{
  for(int yi(1); yi != tc1->ny - 1; yi++)
  {
    for(int xi(1); xi != tc1->nx - 1; xi++)
    {
      double x = (double) xi * tc1->dx;
      double y = (double) yi * tc1->dy;
      tc1->T[xi + yi * tc1->nx] = gauss(x, y, xc, yc, wx, wy);
    }
  }
}

int main()
{
  int Nx=1025;
  int Ny=1024;
  double wx=0.05;
  double wy=0.07;
  double cx1=0.5;
  double cy1=0.6;
  double cx2=0.3;
  double cy2=0.4;
  int N=1000;

  tc* tc1 = new tc(Nx, Ny, 1.0, 1.0);
  tc* tc2 = new tc(Nx, Ny, 1.0, 1.0);
  {
    set(tc1, cx1, cy1, wx, wy);
    set(tc2, cx2, cy2, wx, wy);

    auto start_time = std::chrono::high_resolution_clock::now();
    for(int i=0; i != N; i++)
		{
      step1((void*) tc1);
      step2((void*) tc2);
    }
		auto end_time = std::chrono::high_resolution_clock::now();
    std::cout << "serial took " << 1e-6 * (double) std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() << " s\n";
    tc1->dump("t1.dat");
    tc2->dump("t2.dat");
  }
  
  tc* ptc1 = new tc(Nx, Ny, 1.0, 1.0);
  tc* ptc2 = new tc(Nx, Ny, 1.0, 1.0);
  {
    set(ptc1, cx1, cy1, wx, wy);
    set(ptc2, cx2, cy2, wx, wy);

    executor e(2);
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int i=0; i != N; i++)
    {
      e.exec(&step1, (void*) ptc1, (i + 1) % 2);
      e.exec(&step2, (void*) ptc2, i % 2);
      e.sync();
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    std::cout << "parallel took " << 1e-6 * (double) std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() << " s\n";
    ptc1->dump("pt1.dat");
    ptc2->dump("pt2.dat");
    e.finish();
  }
  //compare
  bool ok=true;
  for(int i=0;i!=tc1->nx*tc1->ny;i++)
    if(tc1->T[i]!=ptc1->T[i])
      ok=false;
  for(int i=0;i!=tc2->nx*tc2->ny;i++)
    if(tc2->T[i]!=ptc2->T[i])
      ok=false;
  if(ok)
    std::cout << "all OK!\n";
  else
    std::cout << "ERROR: parallel computed data differs from serial one\n";
  
  delete tc1;
  delete tc2;
  delete ptc1;
  delete ptc2;
  return 0;
}
