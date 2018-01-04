#pragma once
#include "elements.h"
#include "quadtree.h"

struct POLY // closed poly (each point is presented only once)
{
  std::vector<XY*> points;
  std::vector<IEDGE*> edges;
  bool is_inside(XY&/*, FILE* = nullptr*/) const;
  bool is_inside_simple(XY&) const;
  void fill_edges();
  spatial_tree<IEDGE> edgetree;
  void fill_tree();
};


template<>
bool is_inside<IEDGE>(const IEDGE& e, const bounds& b);

template<>
void get_bounds<IEDGE>(const IEDGE& e, bounds& b);
