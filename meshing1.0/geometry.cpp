#include "geometry.h"
#include "elements.h"
#include <set>
#include <iostream>
#include <cmath>

//this function is not robust... when ray is close to box it will say it crosses the box
//criterias 1 and 2 are not enough calculate if ray crosses box.
//criteria 1 is robus for line
//criteria 2 is not robust for ray
static bool if_ray_cross_abox(const XY& p, const point& kn /*normalized direction vector*/, const bounds& b)
{
  point ortho(-kn.y, kn.x);
  auto box = b.generate_box();
  double px_loc(0.0); 
  bool criteria1(false);
  for(unsigned int i = 0; i <= box.size(); i++)
  {
    double x_loc = (box[(i + box.size()) % box.size()] - point(p.x, p.y)) * ortho;
    if(i > 0 && px_loc * x_loc <= 0.0)
    {
      criteria1 = true;
      break;
    }
    px_loc = x_loc;
  }
  if(!criteria1)
    return false;
  bool criteria2(false);
  for(unsigned int i = 0; i != box.size(); i++)
  {
    double y_loc = (box[i] - point(p.x, p.y)) * kn;
    if(y_loc >= 0.0)
      criteria2 = true;
  }
  
  if(criteria2 && criteria1)
    return true;
  return false;
}

// >= 0 returned when ray crosses an edge and returned value is a distance along kn direction
// -1.0 is returned when ray does not cross an edge
// -2.0 is returned in degenerated case : ray slides along edge or ray touches one of edge vertex
static double if_ray_cross_anedge(const XY& p, const point& kn /*normalized direction vector*/, const IEDGE& e)
{
  point ortho(-kn.y, kn.x);
  double y_loc1 = (point(e.p1->x, e.p1->y) - point(p.x, p.y)) * kn;
  double y_loc2 = (point(e.p2->x, e.p2->y) - point(p.x, p.y)) * kn;
  
  if(y_loc1 < .0 && y_loc2 < .0)
    return -1.0;

  double x_loc1 = (point(e.p1->x, e.p1->y) - point(p.x, p.y)) * ortho;
  double x_loc2 = (point(e.p2->x, e.p2->y) - point(p.x, p.y)) * ortho;

  if(x_loc1 * x_loc2 > .0)
    return -1.0;
  
  if(x_loc1 == x_loc2) // degenerated edge ( == actually means that both are strictly zero)
  {
    if(y_loc1 >= 0. || y_loc2 >= 0.)
      return -2.0;
    else
      return -1.0;
  }
  
  if(x_loc1 == .0 && y_loc1 >= .0) // vertex1 is degenerated
    return -2.0;

  if(x_loc2 == .0 && y_loc2 >= .0)  // vertex2 is degenerated
    return -2.0;
    
  //general case
  
  double y = (x_loc2 * y_loc1 - x_loc1 * y_loc2) / (x_loc2 - x_loc1);
  y = x_loc1 == .0 ? y_loc1 : y;
  y = x_loc2 == .0 ? y_loc2 : y;
  
  if(y < .0)
    return -1.0;

  return y;
}


static void traverse_edgetree_with_line(const node<IEDGE>* currentnode, std::vector<node<IEDGE>*>* leaves_ptr, const XY& p, const point& kn)
{
  for(auto ch : currentnode->childs)
  {
    bounds b = ch->node_bounds;
    if(!if_ray_cross_abox(p, kn, b))
      continue;
    if(ch->is_leaf)
    {
      if(ch->items.size() != 0)
        leaves_ptr->emplace_back(ch);
      continue;
    }
    traverse_edgetree_with_line(ch, leaves_ptr, p, kn);
  }
}

bool POLY::is_inside_simple(XY& p, bool verbose) const
{
  if(verbose)
    std::cout << "is_inside_simple::point is (" << p.x << ", " << p.y << ")\n";

  std::vector<IEDGE*> edges2test;
  for(auto it : this->edges)
    edges2test.emplace_back(it);
  //now all edges which should be tested with line is in edges2test
  int crossings(0);
  int count=0;
  point kn(1.0, 0.0);
  while(true)
  {
    crossings = 0;
    bool degenerated(false);
    for(auto e : edges2test)
    {
      double res = if_ray_cross_anedge(p, kn, *e);
      if(res >= .0)
        crossings++;
      if(res == -2.0)
        degenerated = true;
    }

    count++;

    if(!degenerated || count > 100)
      break;

    if(verbose)
    {
      if(degenerated)
        std::cout << "is_inside_simple::\tdegenerated case\n";
    }
    
    double knx = (double) rand() / (double) RAND_MAX;
    kn = point(knx, sqrt(1.0 - knx * knx));
  }
  
  if(verbose)
  {
    if(crossings % 2 == 0)
      std::cout << "is_inside_simple::\tOUT\n";
    else
      std::cout << "is_inside_simple::\tIN\n";
  }  
  //std::cout << count << std::endl;
  if(crossings % 2 == 0)
    return false;
  else
    return true;  
}

bool POLY::is_inside(XY& p, bool verbose/*, FILE* fptr*/) const
{
  static bool warned(false);
  point kn(1.0, 0.0);
  if(!this->edgetree.is_filled)
  {
    //we have to test all edges since tree is now built
    if(!warned)
    {
      std::cout << "WARNING POLY::is_inside(XY&) will be slow since no tree is built for poly. use fill_tree() method for your poly to speed up is_inside(XY&) method" << std::endl;
      warned = true;
    }
    return is_inside_simple(p);
  }

  
  int crossings(0);
  int count(0);
  while(true)
  {
    std::set<IEDGE*> edges2test;
    //select crossing line
    std::vector<node<IEDGE>*> leaves;
    traverse_edgetree_with_line(&(this->edgetree.master), &leaves, p, kn);

    if(verbose)
    {
      std::cout << "is_inside_tree::point is (" << p.x << ", " << p.y << ")\n";
      for(auto nd : leaves)
        std::cout << "is_inside_tree::found leaf node (" << nd->node_bounds.pmin.x << ", " << nd->node_bounds.pmin.y << ")--(" << nd->node_bounds.pmax.x << ", " << nd->node_bounds.pmax.y << "), W=" << nd->node_bounds.pmax.x - nd->node_bounds.pmin.x << ", H=" << nd->node_bounds.pmax.y - nd->node_bounds.pmin.y << "\n";
    }
    //now all leafnodes crossing line is in leaves
    for(auto nd : leaves)
    {
      for(auto ed_ptr : nd->items)
        edges2test.insert(ed_ptr);
    }
    
    //now all edges which should be tested with line is in edges2test
    crossings = 0;
    bool degenerated(false);
    for(auto e : edges2test)
    {
      double res = if_ray_cross_anedge(p, kn, *e);
      if(verbose)
        std::cout << "is_inside_tree::testing edge (" << e->p1->x << ", " << e->p1->y << ")-(" << e->p2->x << ", " << e->p2->y << "), res=" << res << "\n";

      if(res >= .0)
        crossings++;
      if(res == -2.0)
        degenerated = true;
    }
    
    count++;

    if(!degenerated || count > 100)
      break;

    if(verbose)
    {
      if(degenerated)
        std::cout << "is_inside_tree::\tdegenerated case\n";
    }

    
    double knx = (double) rand() / (double) RAND_MAX;
    kn = point(knx, sqrt(1.0 - knx * knx));

  }
  //std::cout << count << std::endl;
  if(verbose)
  {
    if(crossings % 2 == 0)
      std::cout << "is_inside_tree::\tOUT\n";
    else
      std::cout << "is_inside_tree::\tIN\n";
  }

  if(crossings % 2 == 0)
    return false;
  else
    return true;
}

void POLY::fill_edges()
{
  int pt_idx(0);
  for(; pt_idx != this->points.size()-1; pt_idx++)
    this->edges.push_back(newIEDGE(this->points[pt_idx], this->points[pt_idx+1]));
  this->edges.push_back(newIEDGE(this->points[pt_idx], this->points[0]));
  return;
}

template<>
bool is_inside<IEDGE>(const IEDGE& e, const bounds& b)
{
  bounds eb;
  get_bounds(e, eb);
  if(!eb.cross(b))
    return false;

  double y_xmin = (e.p1->y == e.p2->y) ? e.p1->y : (e.p1->y * e.p2->x - e.p2->y * e.p1->x + b.pmin.x * (e.p2->y - e.p1->y)) / (e.p2->x - e.p1->x);
  double y_xmax = (e.p1->y == e.p2->y) ? e.p1->y : (e.p1->y * e.p2->x - e.p2->y * e.p1->x + b.pmax.x * (e.p2->y - e.p1->y)) / (e.p2->x - e.p1->x);
  double x_ymin = (e.p1->x == e.p2->x) ? e.p1->x : (e.p1->x * e.p2->y - e.p2->x * e.p1->y + b.pmin.y * (e.p2->x - e.p1->x)) / (e.p2->y - e.p1->y);
  double x_ymax = (e.p1->x == e.p2->x) ? e.p1->x : (e.p1->x * e.p2->y - e.p2->x * e.p1->y + b.pmax.y * (e.p2->x - e.p1->x)) / (e.p2->y - e.p1->y);

  if( (y_xmin - b.pmin.y) * (y_xmin - b.pmax.y) <= 0.0 ||
      (y_xmax - b.pmin.y) * (y_xmax - b.pmax.y) <= 0.0 ||
      (x_ymax - b.pmin.x) * (x_ymax - b.pmax.x) <= 0.0 ||
      (x_ymax - b.pmin.x) * (x_ymax - b.pmax.x) <= 0.0 )
    return true;

  return false;
}

template<>
void get_bounds<IEDGE>(const IEDGE& e, bounds& b)
{
  b.pmin = point(std::min(e.p1->x, e.p2->x), std::min(e.p1->y, e.p2->y));
  b.pmax = point(std::max(e.p1->x, e.p2->x), std::max(e.p1->y, e.p2->y));
}

void POLY::fill_tree()
{
  if(edges.size()==0)
    fill_edges();
  edgetree.fill_tree(edges);
}
