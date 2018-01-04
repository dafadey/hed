#pragma once
#include "geometry.h"
#include "elements.h"
#include "searchgrid.h"

struct mesher
{
  double xmin, ymin, xmax, ymax;
  std::vector<POLY> contours; /*input*/
  std::vector<std::array<XY*, 2>> fixed_edges; /*intermediate structure that holds seeded geometry*/
  search_grid<XY> points_sg; // an internal optimizing structure to hold points
  
  std::vector<XY*> nodes; /*output*/
  size_t n_fixed_nodes;
  std::vector<IEDGE*> edges; /*output*/
  std::vector<ITRIANGLE*> triangles; /*output*/
  void seed_geometry(double d);
  void seed_volume(double d);
  void improve_seeding(double d, double dt, bool debug=false);
  void triangulate();
  void build_edges();
  void clear_mesh();
  const XY* sg_get_point(const XY&) const;
  std::array<double, 8> check_quality();
  void print_quality();
};
