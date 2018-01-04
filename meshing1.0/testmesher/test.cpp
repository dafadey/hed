#include <triangulate.h>
#include <mesh.h>
#include <svg.h>
#include <fstream>

int main()
{
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
  mesher m;
  
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
  
  for(int i=0; i!=15; i++)
  {
    for(int j=0; j!=7; j++)
      m.improve_seeding(seed,seed*0.1);
    m.clear_mesh();
    m.triangulate();
    m.print_quality();
  }

  of.open("mesh.debug", std::ios_base::app);
  for(const auto& t : m.triangles)
    of << IEDGE(t->p1, t->p2) << IEDGE(t->p2, t->p3) << IEDGE(t->p3, t->p1);
  of.close();

  if(check_mesh(m.nodes, m.triangles, m.edges))
    std::cout << "MESH IS OK!\n";
  else
    std::cout << "MESH IS BAD!\n";

  //m.improve_seeding(3.0,2.5,true);

  
  of.open("mesh.debug", std::ios_base::app);
//  for(const auto& t : m.triangles)
//    of << *t;
  for(const auto& fe : m.fixed_edges)
  {
    //of << IEDGE(fe[0],fe[1]);
    of << *fe[0];
    of << *fe[1];
  }
  of.close();

  return 0;
}
