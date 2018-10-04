/*
 * this is example implementation for 1d EMP resonatior spectra
 * 
 * 1. ELECTRODYNAMICS
 * 
 * equations are:
 * dE/dt = dB/dx
 * dB/dt = dE/dx
 * 
 * 2. SOME LINEAR ALGEBRA
 * 
 * in terms of linear algebra it reads as follows
 * 
 * dV/dt = L V            (1)
 * 
 * where
 * 
 *     / E \        / 0   d/dx \
 * V = |   |    L = |          |
 *     \ B /        \ d/dx   0 /
 * 
 * with zero boundary conditions for E it has first eigenvalue
 * l0 = i * pi / L
 * and eigenvector
 *      /   sin pi/L * x  \
 * V0 = |                 |
 *      \ -i cos pi/L * x /
 * 
 * with complex time ie
 * dV/dt = i L V            (2)
 * new L` = i L will have same eigenvector but real negative eigen value
 * l0 = - pi / L
 * 
 * 3.COMPLEX TIME METHOD
 * 
 * which allows to run iterative diagonalization procedure by simple
 * advancing (2) with euller's scheme or so.
 * being normalized at each step first eigenvectors expected to survive
 * among others in solution which is apparantly a linear combination of
 * all eigenvectors.
 * finding next eigenvector is of the question since it is not guaranteed
 * to have another parity so it could be hard to withdraw first eigenvalue
 * from solution each time. Also eigenvectors may be non-orthogonal.
 */

#include <heds/solver.h>
#include <svg.h>
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <cmath>
#include <complex>
#include <set>
#include <limits>

typedef std::complex<double> cplx;

struct resonator
{
  resonator() : E(), B(), m(), wgts(), hedsol() {}
  
  resonator(const std::string& input_geo_file) : E(), B(), m(), wgts()
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
    
    double seed=7.0;
    
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

    for(size_t p_i(0); p_i != m.nodes.size(); p_i++)
    {
      auto p = m.nodes[p_i];
      std::cout << "* " << p_i << " * " << p->edges.size() << ' ';
      for(auto e : p->edges)
        std::cout << e->id << ' ';
      std::cout << std::endl;
    }

    of.open("mesh.debug", std::ios_base::app);
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
          of << c << " -1";
        else
          of << c << " 1";

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

    
    for(size_t p_i(0); p_i != m.nodes.size(); p_i++)
    {
      auto p = m.nodes[p_i];
      std::cout << "% " << p_i << " % " << p->edges.size() << ' ';
      for(auto e : p->edges)
        std::cout << e->id << ' ';
      std::cout << std::endl;
    }
      
    m.orient();
    std::cout << "orient is done\n";

    m.build_edges();
    std::cout << "edges are built\n";
    m.fill_ids();
    std::cout << "ids are added to elements\n";

    of.open("mesh.debug", std::ios_base::app);
    for(size_t n_id=0; n_id != m.nodes.size(); n_id++)
    {
      //std::cout << n_id << std::endl;
      of << *(m.nodes[n_id]) << " " << (n_id==54 ? -1 : 0);
    }
    size_t ntimes54(0);
    for(size_t e_id=0; e_id != m.edges.size(); e_id++)
    {
      //std::cout << e_id << std::endl;
      of << *(m.edges[e_id]) << " " << (e_id==275 || e_id==276 ? -1 : 0);
    }
    for(size_t t_id=0; t_id != m.triangles.size(); t_id++)
    {
      //std::cout << t_id << std::endl;
      of << *(m.triangles[t_id]) << " " << (t_id==165 || t_id==178 || t_id==181 ? -1 : 0);
    }
    
    for(auto& fe : m.fixed_edges)
      of << IEDGE(fe[0], fe[1]) << " 1";
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
  }

  std::vector<cplx> E;
  std::vector<cplx> B;
  mesh m;
  weights wgts;
  hed_data hedsol;
  
  std::vector<size_t> bndry;
  
  double norm()
  {
    double n2(.0);
    for(const auto& e : E)
      n2 = std::max(n2, std::norm(e));
    for(const auto& b : B)
      n2 = std::max(n2, std::norm(b));
    n2 = 1.0 / sqrt(n2);
    for(auto& e : E)
      e *= n2;
    for(auto& b : B)
      b *= n2;
    return log(n2);
  }
  
  double step(const cplx& dt)
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
    
    return norm();
  }
  
  resonator& operator -= (const resonator& r)
  {
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
    
    return *this;
  }
  
  resonator& operator = (const resonator& r)
  {
    wgts = r.wgts;
    m = r.m;
    
    E.clear();
    B.clear();
    
    hedsol.m = &m;
    hedsol.w = &wgts;
    
    //hedsol.extract_contour_edges();
    
    for(auto& e : r.E)
      E.emplace_back(e);
    for(auto& b : r.B)
      B.emplace_back(b);
    return *this;
  }
};

std::ostream& operator << (std::ostream& os, const resonator& r)
{
  os << r.E.size() << '\n';
  for(const auto& e : r.E)
    os << e.real() << '\t' << e.imag() << '\n';
  os << r.B.size() << '\n';
  for(const auto& b : r.B)
    os << b.real() << '\t' << b.imag() << '\n';
  return os;
}

int main(int argc, char* argv[])
{
  std::string filename="u_resonator.svg";
  if(argc>2)
    filename=std::string(argv[2]);
  resonator r(filename);
  
  std::cout << "contour edges extracted\n";
  
  std::vector<double> edg_mask;
  edg_mask.resize(r.m.edges.size());
  for(int i=0; i!=r.m.edges.size(); i++)
    edg_mask[i] = .0;
  
  for(size_t i(0); i != r.hedsol.contour_edges.size(); i++)
  {
    for(size_t j(0); j != r.hedsol.contour_edges[i].size(); j++)
      edg_mask[r.hedsol.contour_edges[i][j]] = (double) (i + 1) / (double) r.hedsol.contour_edges.size();
  }
  
  
  //test copy of resonator object
  {
    resonator r2;
    r2=r;
    for(size_t i(0); i!= r.wgts.tris.size(); i++)
    {
      if(r.wgts.tris[i][0] != r2.wgts.tris[i][0])
      {
        std::cerr << "copy of resonators failed" << std::endl;
        return -1;
      }
    }
    
    std::cout << "so far so good!" << std::endl;
  }
  
  std::vector<size_t> outer_edges;
  for(size_t e_i(0); e_i != r.m.edges.size(); e_i++)
  {
    auto e = r.m.edges[e_i];
    if(e->t1 == nullptr || e->t2 == nullptr)
      outer_edges.emplace_back(e_i);
  }
  
  r.bndry = outer_edges;
  
  {
    std::ofstream of("mesh.debug", std::ios_base::app);
    for(auto e_i : r.bndry)
      of << *r.m.edges[e_i] << ' ' << -1;
    of.close();
  }
  
  double min_edge(std::numeric_limits<double>::max());
  for(auto e_ptr : r.m.edges)
  {
    double x = e_ptr->p1->x - e_ptr->p2->x;
    double y = e_ptr->p1->y - e_ptr->p2->y;
    min_edge = std::min(min_edge, sqrt(x * x + y * y));
  }
  
  int n=argc >= 2 ? atoi(argv[1]) : 0;
  cplx dt(.0, (argc>3 ? atof(argv[3]) : 0.05) * min_edge);
  std::cerr << "doing " << n << " steps\n";
  std::cerr << "with dt=" << dt << '\n';
  double l(.0);
  
  for(auto& e : r.E)
    e = cplx((double) rand() / (double) RAND_MAX, .0);
  //for(auto& b : r.B)
  //  b = cplx(.0, (double) rand() / (double) RAND_MAX);
  
  std::vector<resonator> rs(r.B.size());
  for(int i = 0; i != rs.size(); i++)
  {
    rs[i] = r;
    rs[i].bndry = r.bndry;
    rs[i].E = r.E;
    rs[i].B = r.B;
  }
  
  std::vector<double> lambdas(rs.size());
  
  std::ofstream lf("lambdas.dat");
  lf << lambdas.size() << '\n';

  for(int i = 0; i != 10; i++)
  {
    for(int j = 0; j < rs.size(); j++)
    {
      for(int q = 0; q < j; q++)
        rs[j] -= rs[q];
      lambdas[j] = rs[j].step(dt);
    }
  }
  
  for(int i = 0; i != n; i++)
  {
/*
    for(int j = 0; j < rs.size(); j++)
    {
      for(int q = 0; q < j; q++)
        rs[j] -= rs[q];
      lambdas[j] = rs[j].step(dt);
    }
*/
    #pragma omp parallel for
    for(int j = 0; j < rs.size(); j++)
      lambdas[j] = rs[j].step(dt);

    #pragma omp parallel for
    for(int j = 0; j < rs.size(); j++)
    {
      for(int q = 0; q < j; q++)
        rs[j] -= rs[q];
    }

    for(int j = 0; j < rs.size(); j++)
      lf << lambdas[j] << '\n';

    std::cout << i << '/' << n << '\n';
  }
  
  lf.close();
  
  for(const auto& _l : lambdas)
    std::cout << _l << '\n';
 
  {
    for(size_t rsi(0); rsi != rs.size(); rsi++)
    {
      std::ofstream of(("fields"+std::to_string(rsi)+".debug").c_str());//, std::ios_base::app);
      for(size_t e_i(0); e_i != rs[rsi].m.edges.size(); e_i++)
        of << *rs[rsi].m.edges[e_i] << ' ' << rs[rsi].E[e_i].real();
      for(size_t t_i(0); t_i != rs[rsi].m.triangles.size(); t_i++)
        of << *rs[rsi].m.triangles[t_i] << ' ' << rs[rsi].B[t_i].imag();
      of.close();
    }
  }
  
  return 0;
}
