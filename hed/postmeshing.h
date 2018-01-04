#pragma once
#include <vector>
#include <array>

#define TRI_WEIGHTS 1 /* area */
#define EDGE_WEIGHTS 2 /* length, r_left + r_right */
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
