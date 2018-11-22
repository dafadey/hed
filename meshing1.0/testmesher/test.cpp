#include <triangulate.h>
#include <mesh.h>
#include <partition.h>
#include <svg.h>
#include <fstream>
#include <map>

int main(int argc, char* argv[])
{
  // KD/G(greedy) depth/part_size seeding
  if(argc < 3)
  {
    std::cout << "specify at least KD or G(greedy) method to use and size of partition\n";
    return -1;
  }
  std::vector<std::vector<point>> contours;
  svg::importall("input.svg", contours, 0.1);
  std::ofstream of("input.poly");
  int ci(0);
  for(const auto& c : contours)
  {
    of << ci << "\t" << c.size();
    for(const auto& p : c)
      of << "\t" << p.x << "\t" << p.y;
    of << "\n";
    ci++;
  }
  of.close();
  mesh m;
  
  for(const auto& c : contours)
  {
    m.contours.push_back(POLY());
    for(const auto& pt : c)
      m.contours.back().points.push_back(newXY(pt.x, pt.y));
  }
  
  double seed = argc==4 ? atof(argv[3]) : 3.0;
  
  m.seed_geometry(seed);
  std::cout << "seed geometry is done\n";
  m.seed_volume(seed);
  std::cout << "seed volume is done\n";
  m.triangulate();
  std::cout << "triangulate is done\n";

  of.open("mesh.debug");
  of.close();
  
  for(int i=0; i!=13; i++)
  {
    for(int j=0; j!=13; j++)
      m.improve_seeding(seed,seed*0.1);
    m.clear_mesh();
    m.triangulate();
    m.print_quality();
  }



  m.build_edges();
  std::cout << "build edges is done\n";

  if(check_mesh(m.nodes, m.triangles, m.edges))
    std::cout << "MESH IS OK!\n";
  else
    std::cout << "MESH IS BAD!\n";

  //m.improve_seeding(3.0,2.5,true);

  std::cout << "=======+SUMMARY======\n\tnumber of triangles: " << m.triangles.size() << "\n"
            << "\tnumber of edges:" << m.edges.size() << "\n"
            << "\tnumber of nodes:" << m.nodes.size() << "\n";

	m.orient();
	std::cout << "orient is done\n";
	m.build_edges();
	std::cout << "edges are built\n";
	m.fill_ids();
	std::cout << "ids are added to elements\n";

  std::vector<std::vector<const ITRIANGLE*>> parts;

  if(std::string(argv[1]) == "KD")
    parts = KDpartition(m, atoi(argv[2]));
  else if(std::string(argv[1]) == "G")
    parts = greedy_partition(m, atoi(argv[2]));
  else
  {
    std::cout << "method \"" << argv[1] << "\" is not recognized use \"KD\" or \"G\"\n";
    return -1;
  }

  std::map<const ITRIANGLE*, size_t> part_map;
  for(size_t p_id(0); p_id != parts.size(); p_id++)
  {
    for(auto t : parts[p_id])
      part_map[t] = p_id;
  }
  //get boundaries
  std::vector<const IEDGE*> boundaries;
  for(const IEDGE* e : m.edges)
  {
    if(!e->t1 || !e->t2)
      continue;
    if(part_map[e->t1] != part_map[e->t2])
      boundaries.push_back(e);
  }
  std::ofstream pbf("part_boundaries.mesh");
  for(auto e : boundaries)
    pbf << *e << '\n';
  pbf.close();
  

  std::cout << "paritioned to " << parts.size() << " parts\n sizes are";
  for(const auto& p : parts)
    std::cout << " " << p.size();
  std::cout << '\n';

  of.open("mesh.debug", std::ios_base::app);
  for(size_t tris_id(0); tris_id != parts.size(); tris_id++)
  {
    auto& tris = parts[tris_id];
    for(const auto& t : tris)
      of << *t << ' ' << (double) tris_id / (double) (parts.size() - 1) << '\n';
  }
    
  for(const auto& t : m.triangles)
    of << IEDGE(t->p1, t->p2) << '\n' << IEDGE(t->p2, t->p3) << '\n' << IEDGE(t->p3, t->p1) << '\n';
  
  for(const auto& fe : m.fixed_edges)
  {
    //of << IEDGE(fe[0],fe[1]);
    of << *fe[0] << '\n';
    of << *fe[1] << '\n';
  }

  of.close();

  //stats
  double mean = 0;
  for(const auto& p : parts)
    mean += (double) p.size();
  mean /= (double) parts.size();
  double R = 0;
  for(const auto& p : parts)
    R += pow((double) p.size() - mean, 2.0);
  R /= (double) parts.size();
  R = sqrt(R);
  
  std::cout << argv[1] << ' ' << parts.size() << ' ' << boundaries.size() << ' ' << ' ' << mean << ' ' << R << '\n';
  std::ofstream stats("stats.dat", std::ios_base::app);
  stats << argv[1] << ' ' << parts.size() << ' ' << boundaries.size() << ' ' << ' ' << mean << ' ' << R << '\n';
  stats.close();
 
  return 0;
}
