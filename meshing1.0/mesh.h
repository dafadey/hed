#pragma once
#include "geometry.h"
#include "elements.h"
#include "searchgrid.h"
#include <limits>

struct mesh
{
	mesh() : xmin(std::numeric_limits<double>::max()), ymin(std::numeric_limits<double>::max()),
					 xmax(-std::numeric_limits<double>::max()), ymax(-std::numeric_limits<double>::max()),
					 n_fixed_nodes(0), contours(), fixed_edges(), fixed_edges_mask(), points_sg(), nodes(),
           edges(), triangles(), edges_valid(false), oriented(false) {}
  double xmin, ymin, xmax, ymax;
  size_t n_fixed_nodes;
  std::vector<POLY> contours; // input
  std::vector<std::array<XY*, 2>> fixed_edges; // intermediate structure that holds seeded geometry
  std::vector<int> fixed_edges_mask; // maps each of fixed edge to specific contour. size = size of fixed_edges.
  search_grid<XY> points_sg; // an internal optimizing structure to hold points
  std::vector<XY*> nodes; // output
  std::vector<IEDGE*> edges; // output
  std::vector<ITRIANGLE*> triangles; // output
  bool edges_valid;
  bool oriented;
  void seed_geometry(double d);
  void seed_volume(double d);
  void improve_seeding(double d, double dt, bool debug=false);
  void triangulate();
  void build_edges();
	void fill_ids();
  void orient();
  void clear_mesh();
  const XY* sg_get_point(const XY&) const;
  std::array<double, 8> check_quality();
  void print_quality();
};
