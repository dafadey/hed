#pragma once
#include <mesh.h>
#include "postmeshing.h"
#include <map>

struct hed_data
{
  mesh* m;
  weights* w;
  
  std::vector<std::vector<size_t>> contour_edges;
  void extract_contour_edges();
  
  std::vector<hed_data_type> e;
  std::vector<hed_data_type> h;
  std::vector<hed_data_type> j;
  std::vector<hed_data_type> n;
  
  hed_data_type dt;
  
  void calc_e();
  void calc_h();
  void calc_j();
  
  //diagnostics
  std::map<std::pair<ssize_t, ssize_t>, hed_data_type> energy_weights;
  void w_e_weights();
  void w_h_weights();
};

