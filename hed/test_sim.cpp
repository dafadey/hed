#include <heds/solver.h>
#include <svg.h>
#include <fstream>

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
  std::cout << "seed geometry is done\n";
  m.seed_volume(seed);
  std::cout << "seed volume is done\n";
  m.triangulate();
  std::cout << "triangulate is done\n";

  of.open("mesh.debug");
  of.close();
  for(int i=0; i!=3; i++)
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
	
	weights wgts;
	calculate_weights(&wgts, &m);
	std::cout << "weights calculated\n";
	
	hed_data hedsol;
	hedsol.m = &m;
	hedsol.w = &wgts;
  
  hedsol.extract_contour_edges();
	std::cout << "contour edges extracted\n";
  
  of.open("mesh.debug", std::ios_base::app);
  //for(const auto& t : m.triangles)
  //  of << IEDGE(t->p1, t->p2) << " 1" << IEDGE(t->p2, t->p3) << " 1" << IEDGE(t->p3, t->p1) << " 1";
  
  std::vector<double> edg_mask;
  edg_mask.resize(m.edges.size());
  for(int i=0; i!=m.edges.size(); i++)
		edg_mask[i] = .0;
	
  for(size_t i=0; i!=hedsol.contour_edges.size(); i++)
  {
		for(size_t j=0; j!=hedsol.contour_edges[i].size(); j++)
			edg_mask[hedsol.contour_edges[i][j]] = (double) (i + 1) / (double) hedsol.contour_edges.size();
	}

  for(size_t e_id=0; e_id != m.edges.size(); e_id++)
    of << *(m.edges[e_id]) << " " << edg_mask[e_id];
  of.close();

  return 0;
}
