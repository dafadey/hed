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
#include <vector>
#include <cmath>
#include <complex>

typedef std::complex<double> cplx;

struct resonator
{
  resonator() : cells(), E(), B() {}
  
  resonator(int n) : cells(n), E(n+1), B(n)
  {
    for(auto& e : E)
      e = cplx(.0, .0);
    for(auto& b : B)
      b = cplx(.0, .0);
    for(auto& c : cells)
      c = 1.0 / (double) cells.size();
  }

  std::vector<double> cells; // lengths of cells
  std::vector<cplx> E;
  std::vector<cplx> B;

  double norm()
  {
    double n2(.0);
    for(const auto& e : E)
      n2 = std::max(n2, std::norm(e));
    for(const auto& b : B)
      n2 = std::max(n2, std::norm(b));
    n2 = 1.0 / sqrt(n2);
    for(auto& e : E)
      e *= n2;
    for(auto& b : B)
      b *= n2;
    return log(n2);
  }
  
  double step(const cplx& dt)
  {
    for(int i=1 ; i!= cells.size(); i++)
      E[i] += dt * (B[i] - B[i - 1]) / (cells[i-1] + cells[i]);
    for(int i=0 ; i!= cells.size(); i++)
      B[i] += dt * (E[i + 1] - E[i]) / cells[i];
    return norm();
  }
  
  resonator& operator -= (const resonator& r)
  {
    cplx k(.0, .0);
    for(int i(1); i!= E.size()-1; i++)
      k += E[i]*r.E[i]*0.5*(cells[i-1]+cells[i]);
    for(int i(0); i!= B.size(); i++)
      k -= B[i]*r.B[i]*cells[i];
    
    cplx n(.0, .0);
    for(int i(1); i!= E.size()-1; i++)
      n += r.E[i]*r.E[i]*0.5*(cells[i-1]+cells[i]);
    for(int i(0); i!= B.size(); i++)
      n -= r.B[i]*r.B[i]*cells[i];
    
    k /= n;
    for(int i(1); i!= E.size()-1; i++)
      E[i] -= k * r.E[i];
    for(int i(0); i!= B.size(); i++)
      B[i] -= k * r.B[i];
    
    return *this;
  }
  
  resonator& operator = (const resonator& r)
  {
    cells.clear();
    E.clear();
    B.clear();
    for(auto& c : r.cells)
      cells.emplace_back(c);
    for(auto& e : r.E)
      E.emplace_back(e);
    for(auto& b : r.B)
      B.emplace_back(b);
    return *this;
  }
};

std::ostream& operator << (std::ostream& os, const resonator& r)
{
  os << r.E.size() << '\n';
  for(const auto& e : r.E)
    os << e.real() << '\t' << e.imag() << '\n';
  os << r.B.size() << '\n';
  for(const auto& b : r.B)
    os << b.real() << '\t' << b.imag() << '\n';
  return os;
}

int main(int argc, char* argv[])
{
  resonator r(argc>=4 ? atof(argv[3]) : 13);
    
  for(int i=0; i!=r.cells.size(); i++)
  {
    r.cells[i] = (1.0 + ((double) i - (double) (r.cells.size()/2))/(double) r.cells.size() * 0.7) / (double) r.cells.size();
  }
  
  int n=argc >= 2 ? atoi(argv[1]) : 0;
  cplx dt(.0, argc>=3 ? atof(argv[2]) : 1.0);
  std::cerr << "doing " << n << " steps\n";
  std::cerr << "with dt=" << dt << '\n';
  double l(.0);
  for(auto& _l : r.cells)
    l += _l;
  std::cerr << "L=" << l << '\n';

  r.E[r.cells.size()/2]=cplx(1.0, .0);

  /*
    double x(0);
    for(int i=0; i!= r.cells.size();i++)
    {
      if(i>=1)
        r.E[i] = cplx(sin(M_PI * x), .0);
      r.B[i] = cplx(.0, -cos(M_PI * (x + r.cells[i] * 0.5)));
      x += r.cells[i];
    }
  */
  
  std::vector<resonator> rs(r.B.size());
  for(int i = 0; i != rs.size(); i++)
    rs[i] = r;
  
  std::vector<double> lambdas(rs.size());
  
  for(int i = 0; i != n; i++)
  {
    for(int j = 0; j != rs.size(); j++)
    {
      for(int q = 0; q < j; q++)
        rs[j] -= rs[q];
      lambdas[j] = rs[j].step(dt);
    }
  }
  
  for(const auto& _r : rs)
    std::cout << _r;
  
  std::cout << 0 << '\n';
  for(const auto& _l : lambdas)
    std::cout << _l << '\n';
  
  return 0;
}
