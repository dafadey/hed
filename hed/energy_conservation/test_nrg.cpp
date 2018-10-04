#include <heds/solver.h>
#include <svg.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>

int main()
{
  std::vector<std::vector<point>> contours;
  svg::importall("input_geo.svg", contours, 0.1);
  std::ofstream of;

  mesh m;
  
  for(const auto& c : contours)
  {
    m.contours.push_back(POLY());
    for(const auto& pt : c)
      m.contours.back().points.push_back(newXY(pt.x, pt.y));
  }
  
  double seed=3.0;
  
  m.seed_geometry(seed);
  of.open("sg.debug", std::ios_base::app);
  for(auto n : m.nodes)
    of << *n;
  std::cout << "seed geometry is done\n";
  m.seed_volume(seed);
  of.close();

  std::cout << "seed volume is done\n";
  m.triangulate();
  std::cout << "triangulate is done\n";

  of.open("mesh.debug");
  of.close();
  for(int i=0; i!=7; i++)
  {
    for(int j=0; j!=7; j++)
      m.improve_seeding(seed,seed*0.1);
    m.clear_mesh();
    m.triangulate();
    m.print_quality();
  }

	m.orient();
	std::cout << "orient is done\n";
	m.build_edges();
	std::cout << "edges are built\n";
	m.fill_ids();
	std::cout << "ids are added to elements\n";
	
	weights wgts;
	calculate_weights(&wgts, &m);
	std::cout << "weights calculated\n";
	
	hed_data hedsol;
	hedsol.m = &m;
	hedsol.w = &wgts;
  hedsol.n.resize(m.nodes.size());
  for(auto it : hedsol.n)
		it = .0;
  hedsol.e.resize(m.edges.size());
  for(auto it : hedsol.e)
		it = .0;
  hedsol.h.resize(m.triangles.size());
  for(auto it : hedsol.h)
		it = .0;
  hedsol.j.resize(m.edges.size());
  for(auto it : hedsol.j)
		it = .0;
 
  hedsol.extract_contour_edges();
	std::cout << "contour edges extracted\n";
  
  //fill n data
  int metal_contour_id = 1;
  std::set<XY*> nodes_inside_metal;
  POLY& metal_poly = m.contours[metal_contour_id];
  metal_poly.fill_tree();
  std::cout << "filled octree for metal particle contour\n";
  // add what is inside
  for(auto n : m.nodes)
  {
    if(metal_poly.is_inside(*n))
      nodes_inside_metal.insert(n);
  }
  std::cout << "added nodes inside metal contours\n";
  

  hedsol.w_e_weights();
  hedsol.w_h_weights();
    
  of.open("mesh.debug", std::ios_base::app);
  //for(const auto& t : m.triangles)
  //  of << IEDGE(t->p1, t->p2) << " 1" << IEDGE(t->p2, t->p3) << " 1" << IEDGE(t->p3, t->p1) << " 1";
  
  std::vector<double> edg_mask;
  edg_mask.resize(m.edges.size());
  for(int i=0; i!=m.edges.size(); i++)
		edg_mask[i] = .0;
	
  std::cout << "hedsol.energy_weights size is " << hedsol.energy_weights.size() << '\n';
  
  std::map<ssize_t, double> ew;
  for(const auto& it : hedsol.energy_weights)
  {
    if(ew.find(it.first.first) == ew.end())
      ew[it.first.first] = .0;
    if(std::abs(ew[it.first.first]) < std::abs(it.second))
      ew[it.first.first] = it.second;
  }
  
  for(size_t e_id=0; e_id != m.edges.size(); e_id++)
    of << *(m.edges[e_id]) << " " << ew[e_id];
  of.close();

  double se(.0);
  for(const auto& ed : wgts.edgs)
    se += std::abs(ed[0] / ed[1]) * 0.5;

  double st(.0);
  for(const auto& tr : wgts.tris)
    st += std::abs(1.0 / tr[0]);

  std::cout << "se=" << se << ", st=" << st << '\n';
  return 0;
}
