#include "resonators.h"
#include <svg.h>
#include <fstream>
#include <iostream>

unstructured_resonator::unstructured_resonator() : resonator_base(), m(), wgts(), hedsol() {}
  
unstructured_resonator::unstructured_resonator(const std::string& input_geo_file, double seed) : resonator_base(), m(), wgts(), hedsol(), bndry()
{
  std::vector<std::vector<point>> contours;
  svg::importall(input_geo_file, contours, 0.1);
  
  std::ofstream of;

  for(const auto& c : contours)
  {
    m.contours.push_back(POLY());
    for(const auto& pt : c)
      m.contours.back().points.push_back(newXY(pt.x, pt.y));
  }

  m.seed_geometry(seed);
  
  of.open("sg.debug", std::ios_base::app);
  std::cout << "seed geometry is done\n";
  m.seed_volume(seed);
  for(auto n : m.nodes)
    of << *n;
  of.close();

  std::cout << "seed volume is done\n";
  m.triangulate();
  std::cout << "triangulate is done\n";

  for(int i=0; i!=7; i++)
  {
    for(int j=0; j!=7; j++)
      m.improve_seeding(seed,seed*0.1);
    m.clear_mesh();
    m.triangulate();
    m.print_quality();
  }

  of.open("mesh.debug");
  //remove extra triangles
  {
    POLY resonator_poly;
    for(auto pt : m.contours[0].points)
      resonator_poly.points.push_back(newXY(pt->x, pt->y));
    resonator_poly.fill_tree();

    ssize_t t_i = m.triangles.size() - 1;
    for(; t_i != -1; t_i--)
    {
      XY c = m.triangles[t_i]->centroid();
      if(resonator_poly.is_inside(c))
        of << c << " -1" << '\n';
      else
        of << c << " 1" << '\n';

      if(!resonator_poly.is_inside(c))
      {
        std::array<XY*,3> tpts{{m.triangles[t_i]->p1, m.triangles[t_i]->p2, m.triangles[t_i]->p3}};
        for(auto pt : tpts)
        {
          for(ssize_t tt_i(pt->tris.size() - 1); tt_i != -1; tt_i--)
          {
            if(pt->tris[tt_i] == m.triangles[t_i])
              pt->tris.erase(pt->tris.begin()+tt_i);
          }
        }
        //delete m.triangles[t_i];
        m.triangles.erase(m.triangles.begin() + t_i);
      }
    }
    for(ssize_t n_i(m.nodes.size()-1); n_i != -1; n_i--)
    {
      if(m.nodes[n_i]->tris.size() == 0)
        m.nodes.erase(m.nodes.begin() + n_i);
    }
  }
  of.close();

  m.orient();
  std::cout << "orient is done\n";

  m.build_edges();
  std::cout << "edges are built\n";
  m.fill_ids();
  std::cout << "ids are added to elements\n";

  //dump geometry
  of.open("mesh.debug", std::ios_base::app);
  for(size_t n_id=0; n_id != m.nodes.size(); n_id++)
    of << *(m.nodes[n_id]) << " " << 0 << '\n';
  for(size_t e_id=0; e_id != m.edges.size(); e_id++)
    of << *(m.edges[e_id]) << " " << 0 << '\n';
  for(size_t t_id=0; t_id != m.triangles.size(); t_id++)
    of << *(m.triangles[t_id]) << " " << 0 << '\n';
  for(auto& fe : m.fixed_edges)
    of << IEDGE(fe[0], fe[1]) << " 1" << '\n';
  of.close();
  
  calculate_weights(&wgts, &m);
  std::cout << "weights calculated\n";
  
  hedsol.m = &m;
  hedsol.w = &wgts;
  
  //hedsol.extract_contour_edges();
  
  E.resize(m.edges.size());
  B.resize(m.triangles.size());
  
  for(auto& e : E)
    e = cplx(.0, .0);
  for(auto& b : B)
    b = cplx(.0, .0);
    
  //extract contour edges
  bndry.clear();
  for(size_t e_i(0); e_i != m.edges.size(); e_i++)
  {
    auto e = m.edges[e_i];
    if(e->t1 == nullptr || e->t2 == nullptr)
      bndry.emplace_back(e_i);
  }
  std::cout << "contour edges extracted\n";
  //dump them to mesh.debug too
  of.open("mesh.debug", std::ios_base::app);
  for(auto e_i : bndry)
    of << *m.edges[e_i] << ' ' << -1 << '\n';
  of.close();

}

double unstructured_resonator::step(const cplx& dt)
{
  const weights& w = wgts;
  for(int i(0); i != E.size(); i++)
  {
    const ssize_t eid = i; // = m.edges[i].id;
    const ssize_t ir = m.edges[eid]->t1 ? m.edges[eid]->t1->id : -1;
    const ssize_t il = m.edges[eid]->t2 ? m.edges[eid]->t2->id : -1;
    E[i] -= ((ir == -1 ? hed_data_type(.0) : B[ir]) -
             (il == -1 ? hed_data_type(.0) : B[il])) * w.edgs[i][1] * dt;
  }
  
  for(size_t zero_e_id : bndry)
    E[zero_e_id] = cplx(.0, .0);
  
  for(int i(0); i != B.size(); i++)
  {
    const ssize_t tid = i; // = m.triangles[i].id;
    cplx db(.0, .0);
    for(size_t eid_local(0); eid_local != 3; eid_local++)
    {
      const size_t eid = m.triangles[tid]->edges[eid_local]->id;
      db += (m.edges[eid]->t1 == m.triangles[tid] ? hed_data_type(1.0) : hed_data_type(-1.0))
             * E[eid] * w.edgs[eid][0]; // keep same orientation of edge as triangle have
    }
    B[i] += db * dt * w.tris[i][0];
  }

  return norm() / dt.imag();
}

bool unstructured_resonator::is_same(const unstructured_resonator& r) const
{
  if(m.triangles.size() == r.m.triangles.size()) // naive test but anyway...
    return true;
  return false;
}
  
void unstructured_resonator::withdraw(const resonator_base* r_ptr)
{
  const unstructured_resonator* ur_ptr = dynamic_cast<const unstructured_resonator*>(r_ptr);
  if(!ur_ptr)
  {
    std::cerr << "ERROR : you can withdraw only unstructured resonator from unstructured resonator\n";
    assert(false);
  }
  const unstructured_resonator& r = *ur_ptr;
  cplx k(.0, .0);
  for(int i(1); i!= E.size()-1; i++)
    k += E[i] * r.E[i] * wgts.edgs[i][0] / wgts.edgs[i][1];
  for(int i(0); i!= B.size(); i++)
    k -= B[i] * r.B[i] / wgts.tris[i][0];
  
  cplx n(.0, .0);
  for(int i(1); i!= E.size()-1; i++)
    n += r.E[i] * r.E[i] * wgts.edgs[i][0] / wgts.edgs[i][1];
  for(int i(0); i!= B.size(); i++)
    n -= r.B[i] * r.B[i] / wgts.tris[i][0];
  
  k /= n;
  for(int i(0); i!= E.size(); i++)
    E[i] -= k * r.E[i];
  for(int i(0); i!= B.size(); i++)
    B[i] -= k * r.B[i];
}
 
unstructured_resonator& unstructured_resonator::operator = (const unstructured_resonator& r)
{
  wgts = r.wgts;
  m = r.m;
  
  hedsol.m = &m;
  hedsol.w = &wgts;
  
  //hedsol.extract_contour_edges();
  
  copy_fields(&r);
 
  return *this;
}



std::ostream& operator << (std::ostream& os, const unstructured_resonator& r)
{
  os << r.E.size() << '\n';
  for(const auto& e : r.E)
    os << e.real() << '\t' << e.imag() << '\n';
  os << r.B.size() << '\n';
  for(const auto& b : r.B)
    os << b.real() << '\t' << b.imag() << '\n';
  return os;
}
