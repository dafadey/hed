/*
 * after mesh is created some additional structures are needed in order to perform calculations
 * we need to calculate centers of tringle incircles and their radii
 *                      lengths of edges
 *                      see 'weights' struct in postmeshing.h for details
 */

#include <mesh.h>
#include <vectors.h>
#include <svg.h>
#include <fstream>
#include "postmeshing.h"

vec_type get_circumcircle_center(const ITRIANGLE* t)
{
  const double x1_x2 = t->p1->x - t->p2->x;
  const double y1_y2 = t->p1->y - t->p2->y;
  
  const double x2_x3 = t->p2->x - t->p3->x;
  const double y2_y3 = t->p2->y - t->p3->y;
  
  const double x1sqr = t->p1->x * t->p1->x;
  const double x2sqr = t->p2->x * t->p2->x;
  const double x3sqr = t->p3->x * t->p3->x;
  
  const double y1sqr = t->p1->y * t->p1->y;
  const double y2sqr = t->p2->y * t->p2->y;
  const double y3sqr = t->p3->y * t->p3->y;
  
  double x = .5 * (y2_y3 * (x1sqr - x2sqr + y1sqr - y2sqr) -
                   y1_y2 * (y2sqr - y3sqr + x2sqr - x3sqr)) /
                   (x1_x2 * y2_y3 - x2_x3 * y1_y2);

  double y = .5 * (x2_x3 * (x1sqr - x2sqr + y1sqr - y2sqr) -
                   x1_x2 * (y2sqr - y3sqr + x2sqr - x3sqr)) /
                   (y1_y2 * x2_x3 - y2_y3 * x1_x2);
  vec_type res(x, y);
  return res;
}

std::array<double, 2> get_central_heights(const IEDGE* e)
{
  std::array<double, 2> hs;
  vec_type p0(e->p1);
  vec_type p1(e->p2);
  const vec_type c = .5 * (p0 + p1);
  vec_type c1 = get_circumcircle_center(e->t1) - c;
  vec_type c2(.0, .0);
  if(e->t2)
		c2 = get_circumcircle_center(e->t2) - c;
  hs[0] = c1.norm();
  hs[1] = c2.norm();
  return hs;
}

void calculate_weights(weights* w, const mesh* m)
{
  for(auto t : m->triangles)
    w->tris.push_back(std::array<hed_data_type, TRI_WEIGHTS>{{1.0 / t->area()}});
  
  std::cout << "triangle weights are done\n";
  
  for(auto e : m->edges)
  {
    const auto hs = get_central_heights(e);
    const vec_type p1(e->p1);
    const vec_type p2(e->p2);
    w->edgs.push_back(std::array<hed_data_type, EDGE_WEIGHTS>{{(p2 - p1).norm(), 1.0 / (hs[0] + hs[1])}});
  }
   
  for(auto p : m->nodes)
  {
    double s(.0);
    for(auto e : p->edges)
    {
      size_t eid = e->id;
      //std::cout << "***" << eid << '\n';
      s += 0.25 * w->edgs[eid][0] * w->edgs[eid][1];
    }
    w->nds.push_back(std::array<hed_data_type, NODE_WEIGHTS>{{1.0 / s}});
  }

  std::cout << "node weights are done\n";
}
