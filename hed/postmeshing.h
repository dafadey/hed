#pragma once
#include <vector>
#include <array>
#include <vectors.h>
#include <mesh.h>

#define TRI_WEIGHTS 1 /* area */
#define EDGE_WEIGHTS 2 /* length, h_left + h_right */
#define NODE_WEIGHTS 1 /* voronoi area */

struct weights
{
  #ifdef TRI_WEIGHTS
  std::vector<std::array<double, TRI_WEIGHTS>> tris;
  #endif
  #ifdef EDGE_WEIGHTS
  std::vector<std::array<double, EDGE_WEIGHTS>> edgs;
  #endif
  #ifdef NODE_WEIGHTS
  std::vector<std::array<double, NODE_WEIGHTS>> nds;
  #endif
};

vec_type get_circumcircle_center(const ITRIANGLE*);

std::array<double, 2> get_central_heights(const IEDGE*);

void calculate_weights(weights* w, const mesher* m);
