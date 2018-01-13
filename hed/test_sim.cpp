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
  hedsol.e.resize(m.edges.size());
  for(auto it : hedsol.e)
		it = .0;
  hedsol.h.resize(m.triangles.size());
  for(auto it : hedsol.h)
		it = .0;
  
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

	hedsol.dt = 0.1;
	hed_data_type t = .0;
	size_t target_c_id = hedsol.contour_edges.size() - 1;
	int step = 0;

	while(t < hed_data_type(100.0))
	{
		for(size_t j=0; j!=hedsol.contour_edges[target_c_id].size(); j++)
		{
			size_t e_id = hedsol.contour_edges[target_c_id][j];
			hed_data_type dir = m.edges[e_id]->p2->x > m.edges[e_id]->p1->x ? hed_data_type(1.0) : hed_data_type(-1.0); 
			hedsol.e[e_id] += dir * sin(t*.1) * hedsol.dt;
		}
		hedsol.calc_e();
		hedsol.calc_h();
    if(step%10 == 0)
    {
      of.open(("fields/fields"+std::to_string(step)+".debug").c_str());
      for(size_t e_id=0; e_id != m.edges.size(); e_id++)
        of << *(m.edges[e_id]) << " " << hedsol.e[e_id];
      for(size_t t_id=0; t_id != m.triangles.size(); t_id++)
        of << *(m.triangles[t_id]) << " " << hedsol.h[t_id];
      of.close();
    }
		step++;
		t += hedsol.dt;
	}
	

  return 0;
}
