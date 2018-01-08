#include "postmeshing.h"
#include <fstream>
#include <vector>
#include <cmath>

int main()
{
  auto p1 = newXY(-1.0, -2.0);
  auto p2 = newXY(3.0, -0.5);
  auto p3 = newXY(1.5, 2.0);
  auto t = newITRIANGLE(p1, p2, p3);
  vec_type c = get_circumcircle_center(t);
  double R = (c - vec_type(p1)).norm();
  XY* cp = newXY(c[0], c[1]);
  
  std::vector<IEDGE*> circle_edges;
  for(int i=0; i!=360; i++)
  {
    double alpha0 = (double) i / 180.0 * M_PI;
    vec_type p0 = c + vec_type(R * cos(alpha0), R * sin(alpha0));
    double alpha1 = (double) (i + 1) / 180.0 * M_PI;
    vec_type p1 = c + vec_type(R * cos(alpha1), R * sin(alpha1));
    circle_edges.push_back(newIEDGE(newXY(p0[0], p0[1]), newXY(p1[0], p1[1])));
  }
  std::ofstream of("test.dat");
  of << *t;
  of << *cp;
  for(const auto& e : circle_edges)
    of << *e;
  of.close();
  return 0;
}
