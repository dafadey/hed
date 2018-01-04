#include "mesh.h"
#include "triangulate.h"
#include <map>

static double length2(const XY& a, const XY& b)
{
  return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

/* the below function is required for search grid structure */
template<>
void search_grid<XY>::get_bounds(const XY& p, bounds& b)
{
  b.pmin.x = p.x; b.pmin.y = p.y;
  b.pmax.x = p.x; b.pmax.y = p.y;
}

const XY* mesher::sg_get_point(const XY& p) const
{
  for(auto p_ptr : points_sg.get_cell(p.x, p.y))
  {
    if(p_ptr->x == p.x && p_ptr->y == p.y)
      return p_ptr;
  }
  return nullptr;
}

double ranged_rand(double v_min, double v_max)
{
  return ((double) rand() / (double) RAND_MAX * (v_max - v_min)) + v_min;
}

void mesher::seed_geometry(double d)
{
  const bool debug(true);
  xmin = std::numeric_limits<double>::max();
  xmax = - xmin;
  ymin = xmin;
  ymax = - xmin;
  for(const auto& c : contours)
  {
    for(auto p : c.points)
    {
      nodes.push_back(p);
      xmin = std::min(xmin, p->x);
      xmax = std::max(xmax, p->x);
      ymin = std::min(ymin, p->y);
      ymax = std::max(ymax, p->y);
    }
    if(*(c.points[0]) == *(c.points[c.points.size() - 1]))
      nodes.pop_back();
  }
  points_sg.OPC=-1;
  points_sg.nx=100;
  points_sg.ny=100;
  
  if(debug)
    std::cout << "constructing search grid\n";
  points_sg.fill(nodes);
  if(debug)
  {
    std::cout << "search grid is constructed\n";
    std::ofstream of("sg.debug");
    of << points_sg;
    of.close();
  }
  
  for(const auto& c : contours)
  {
    bool closed(false);
    if(*(c.points[0]) == *(c.points[c.points.size() - 1]))
      closed = true;
    for(size_t id(0); id != c.points.size() - (closed ? 0 : 1); id++)
    {
      size_t id1 = id == c.points.size() - 1 ? 0 : id + 1;
      const double L = sqrt(length2(*(c.points[id]), *(c.points[id1])));
      const double n = L / d;
      const int nsub(ceil(n));
      const double rd = L / double(nsub);
      std::vector<const XY*> subdivnodes;
      subdivnodes.resize(nsub + 1);
      subdivnodes[0] = sg_get_point(*(c.points[id]));
      subdivnodes[nsub] = sg_get_point(*(c.points[id1]));
      if(!(subdivnodes[0] && subdivnodes[nsub]))
      {
        std::cerr << "critical failure for edge with id " << id << "\n";
        continue;
      }
      for(size_t i(1); i < nsub; i++)
      {
        double x = c.points[id]->x + (double) i * rd / L * (c.points[id1]->x - c.points[id]->x);
        double y = c.points[id]->y + (double) i * rd / L * (c.points[id1]->y - c.points[id]->y);
        subdivnodes[i] = newXY(x, y);
        points_sg.add_one(subdivnodes[i]);
        nodes.push_back((XY*) subdivnodes[i]);
      }
      for(size_t i(0); i != nsub; i++)
        fixed_edges.push_back(std::array<XY*, 2> {{(XY*) subdivnodes[i], (XY*) subdivnodes[i + 1]}});
    }
  }
  n_fixed_nodes = nodes.size();
  if(debug)
  {
    std::ofstream of("sg.debug", std::ios_base::app);
    for(const auto& e : fixed_edges)
      of << IEDGE(e[0], e[1]);
    of.close();
  }
}

void mesher::seed_volume(double d)
{
  //push some points
  const double volume = (xmax - xmin) * (ymax - ymin);
  const double tri_volume = 0.5 * d * d * sin(M_PI/3.0);
  int vsn =  volume / tri_volume / 2 - nodes.size(); // volume seed points number
  for(int i(0); i != vsn; i++)
    nodes.push_back(newXY(ranged_rand(xmin,xmax), ranged_rand(ymin,ymax)));
}

void mesher::triangulate()
{
  const bool debug(false);
  std::vector<XY*> pts;
  for(auto n : nodes)
    pts.push_back(n);
  Triangulate(pts, triangles, fixed_edges);
  
  if(debug)
  {
    std::ofstream of("sg.debug", std::ios_base::app);
    for(const auto& t : triangles)
      of << *t;
    of.close();
  }
  
}

void mesher::clear_mesh()
{
  for(auto n : nodes)
  {
    n->tris.clear();
    n->edges.clear();
  }
  for(auto t : triangles)
    delete t;
  for(auto e : edges)
    delete e;
  triangles.clear();
  edges.clear();
}

struct vec_type : public std::array<double, 2>
{
  vec_type() : std::array<double, 2>{{0,0}} {}
  vec_type(const vec_type& v) : std::array<double, 2>{{v[0], v[1]}} {}
  vec_type(double x, double y) : std::array<double, 2>{{x, y}} {}
  vec_type(const XY* ppt) : std::array<double, 2>{{ppt->x, ppt->y}} {}
  
  
  vec_type operator+(const vec_type& v)
  {
    vec_type res(*this);
    res[0] += v[0];
    res[1] += v[1];
    return res;
  }
  vec_type operator-(const vec_type& v)
  {
    vec_type res(*this);
    res[0] -= v[0];
    res[1] -= v[1];
    return res;
  }
  vec_type operator*(double a)
  {
    vec_type res(*this);
    res[0] *= a;
    res[1] *= a;
    return res;
  }
  double norm2()
  {
    return (*this)[0] * (*this)[0] + (*this)[1] * (*this)[1];
  }
  double norm()
  {
    return sqrt((*this)[0] * (*this)[0] + (*this)[1] * (*this)[1]);
  }
};

double vp(const vec_type& p0, const vec_type& p1)
{
	return p0[0] * p1[1] - p0[1] * p1[0];
}
double dp(const vec_type& p0, const vec_type& p1)
{
	return p0[0] * p1[0] + p0[1] * p1[1];
}

std::ostream& operator<<(std::ostream& s, const vec_type& v)
{
  return s << " " << v[0] << " " << v[1];
}

static std::array<XY*,2> getOppositeVerts(ITRIANGLE* t, XY* n)
{
  XY* tnodes[]{t->p1, t->p2, t->p3};
  int j(0);
  XY* oe[3];
  for(int i(0); i != 3; i++)
  {
    if(tnodes[i] == n)
      continue;
    oe[j]=tnodes[i];
    j++;
  }
  if(j==3)
  {
    std::cerr << "WARNING: cannot find opposite edge\n";
    return std::array<XY*,2>{{nullptr, nullptr}};
  }
  return std::array<XY*,2>{{oe[0], oe[1]}};
}

static vec_type getH(ITRIANGLE* t, XY* n)
{
  auto oe = getOppositeVerts(t, n);
  vec_type vn(n->x, n->y);
  vec_type oe0(oe[0]->x, oe[0]->y);
  vec_type oe1(oe[1]->x, oe[1]->y);
  
  double a2 = (vn - oe0).norm2();
  double b2 = (oe1 - oe0).norm2();
  double c2 = (vn - oe1).norm2();
  // cos = (a2 + b2 - c2) * 0.5 / sqrt(a2) / sqrt(b2)
  // (cos * sqrt(a2) / sqrt(b2)) * vec_b + vec_oe0 = vec_h 
  double l = (a2 + b2 - c2) * 0.5 / b2;
  return oe0 + (oe1 - oe0) * l;
  
}

double force(double r2, double r2_0)
{
  double r = sqrt(r2);
  double r0 = sqrt(r2_0);
  double f = 1.0 - r / r0;
  f = f < -1.0 ? -1.0 : f;
  return f;
}

void mesher::improve_seeding(double d, double dt, bool debug)
{
  //double dt = 0.1;
  //std::cout << "has " << n_fixed_nodes << " fixed nodes\n";
  
  std::vector<vec_type> shifts;
  shifts.resize(nodes.size());

  std::ofstream of("mesh.debug", std::ios_base::app);

  for(size_t nd_id(n_fixed_nodes); nd_id != nodes.size(); nd_id++)
  {
    auto n = nodes[nd_id];
    vec_type f(.0,.0);
    for(auto t : n->tris)
    {
      vec_type nv(n);

      if(debug)
      {
        vec_type a(t->p1);
        vec_type b(t->p2);
        vec_type c(t->p3);

        of << "t" << nv + (a - nv) * 0.25 << nv + (b - nv) * 0.25 << nv + (c - nv) * 0.25 << "\n";
      }
      
      vec_type h = getH(t, n);
      if((h - nv).norm2() < d * d)
        f = f - (h - nv) * (force((h - nv).norm2(), d * d * 0.75) / (h - nv).norm());
      
      auto oe = getOppositeVerts(t, n);
      for(auto& on : oe)
      {
        vec_type ov(on);
        f = f - (ov - nv) * (force((ov - nv).norm2(), d * d) / (ov - nv).norm());
        if(debug)
          of << "e" << nv << nv + (ov - nv) * 0.45 << "\n";
      }
    }
    shifts[nd_id] = vec_type(f[0] * dt, f[1] * dt);
  }
    //std::cout << "moving by " << sqrt(f.norm2()) * dt << "\n";
  for(size_t nd_id(n_fixed_nodes); nd_id != nodes.size(); nd_id++)
  {
    auto n = nodes[nd_id];
    if(debug)
      of << "e " << n->x << " " << n->y << " " << n->x + shifts[nd_id][0] << " " << n->y + shifts[nd_id][1] << "\n";
    else
    {
      const double ds = d * 0.03;
      n->x += shifts[nd_id][0];
      n->y += shifts[nd_id][1];
      n->x = (n->x < xmin + ds) ? xmin + ds : n->x;
      n->x = (n->x > xmax - ds) ? xmax - ds : n->x;
      n->y = (n->y < ymin + ds) ? ymin + ds : n->y;
      n->y = (n->y > ymax - ds) ? ymax - ds : n->y;
    }
  }
  of.close();
}

std::array<double, 8> mesher::check_quality()
{
  double savg(.0), ds(.0), smin(std::numeric_limits<double>::max()), smax(-std::numeric_limits<double>::max());
  double lavg(.0), dl(.0), lmin(std::numeric_limits<double>::max()), lmax(-std::numeric_limits<double>::max());
  for(auto t : triangles)
  {
    vec_type a = vec_type(t->p2) - vec_type(t->p1);
    vec_type b = vec_type(t->p3) - vec_type(t->p2);
    vec_type c = vec_type(t->p1) - vec_type(t->p3);
    double l[3]{a.norm(), b.norm(), c.norm()};
    double s = std::abs(0.5 * vp(a, c));
    savg += s;
    for(int i(0); i != 3; i++)
    {
      lavg += l[i];
      lmin = std::min(lmin, l[i]);
      lmax = std::max(lmax, l[i]);
    }
    smin = std::min(smin, s);
    smax = std::max(smax, s);
  }
  savg /= (double) triangles.size();
  lavg /= 2.0 * (double) triangles.size();

  for(auto t : triangles)
  {
    vec_type a = vec_type(t->p2) - vec_type(t->p1);
    vec_type b = vec_type(t->p3) - vec_type(t->p2);
    vec_type c = vec_type(t->p1) - vec_type(t->p3);
    double l[3]{a.norm(), b.norm(), c.norm()};
    double s = std::abs(0.5 * vp(a, c));
    ds += std::abs(s - savg);
    for(int i(0); i != 3; i++)
      dl += std::abs(lavg - l[i]);
  }
  ds /= (double) triangles.size();
  dl /= 2.0 * (double) triangles.size();
  return std::array<double, 8>{{savg, ds, smin, smax, lavg, dl, lmin, lmax}};
}

void mesher::print_quality()
{
  auto q = check_quality();
  std::cout << "quality: savg=" << q[0] << ", ds=" << q[1] << ", smin=" << q[2] << ", smax=" << q[3] << "\n"
            << "         lavg=" << q[4] << ", dl=" << q[5] << ", lmin=" << q[6] << ", lmax=" << q[7] << "\n";
}


void mesher::build_edges()
{
  typedef std::array<XY*, 2> edge_graph_type;
  std::map<edge_graph_type, IEDGE*> emap;
  for(auto t : triangles)
  {
    #define MAKE_ORDERED_PAIR(X, Y) (edge_graph_type{{std::min(X, Y), std::max(X, Y)}})
    const edge_graph_type tedges[3]{MAKE_ORDERED_PAIR(t->p1, t->p2),
                                        MAKE_ORDERED_PAIR(t->p2, t->p3),
                                        MAKE_ORDERED_PAIR(t->p3, t->p1)};
    for(const auto& e : tedges)
    {
      auto it = emap.find(e);
      if(it != emap.end())
        it->second->t2 = t;
      else
      {
        IEDGE* newe = newIEDGE(e[0], e[1]);
        newe->t1 = t;
        emap[e] = newe;
        edges.push_back(newe);
      }
      
    }
    #undef MAKE_ORDERED_PAIR
  }
  for(auto e : edges)
  {
    
  }
}
