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
  // add contour points
  for(size_t e_id : hedsol.contour_edges[metal_contour_id])
  {
    nodes_inside_metal.insert(m.edges[e_id]->p1);
    nodes_inside_metal.insert(m.edges[e_id]->p2);
  }
  std::cout << "added nodes on the metal contours\n";
  // apply some electron density to those nodes inside metall
  for(auto n : nodes_inside_metal)
    hedsol.n[n->id] = 0.3;
  std::cout << "added electron density\n";
  
  
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
  
  hed_data_type Tmax(200.0);
  
  int num_digits_in_filename = ceil(log10(Tmax/hedsol.dt));
  
	while(t < Tmax)
	{
		for(size_t j=0; j!=hedsol.contour_edges[target_c_id].size(); j++)
		{
			size_t e_id = hedsol.contour_edges[target_c_id][j];
			hed_data_type dir = m.edges[e_id]->p2->x > m.edges[e_id]->p1->x ? hed_data_type(1.0) : hed_data_type(-1.0); 
			hedsol.e[e_id] += hed_data_type(0.5) * dir * sin(t*.1) * hedsol.dt;
		}
		hedsol.calc_e();
		hedsol.calc_h();
		hedsol.calc_j();
    if(step%10 == 0)
    {
      hed_data_type norme(.0);
      for(auto e : hedsol.e)
        norme += std::abs(e);
      norme /= (hed_data_type) hedsol.e.size();
      hed_data_type normh(.0);
      for(auto h : hedsol.h)
        normh += std::abs(h);
      normh /= (hed_data_type) hedsol.h.size();
      hed_data_type normj(.0);
      for(auto j : hedsol.j)
        normj += std::abs(j);
      normj /= (hed_data_type) hedsol.j.size();
      std::cout << "step = " << step << " |e|=" << norme << " |h|=" << normh << " |j|=" << normj << "\n";
      if(norme > (hed_data_type) 10 || normh > (hed_data_type) 10 || normj > (hed_data_type) 10)
      {
        std::cerr << "uuups solution exploded\n";
        return 0;
      }
      std::stringstream filename;
      filename << "fields/fields" << std::setw(num_digits_in_filename) << std::setfill('0') << step << ".debug";
      of.open(filename.str().c_str());
      for(size_t n_id=0; n_id != m.nodes.size(); n_id++)
        of << *(m.nodes[n_id]) << " " << hedsol.n[n_id];
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
