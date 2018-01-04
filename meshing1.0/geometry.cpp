#include "geometry.h"
#include "elements.h"
#include <set>
#include <iostream>

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
  for(unsigned int i = 0; i != box.size(); i++)
  {
    double x_loc = (box[i] - point(p.x, p.y)) * ortho;
    if(px_loc * x_loc < 0)
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
    if(y_loc > 0)
      criteria2 = true;
  }
  
  if(criteria2 && criteria1)
    return true;
  return false;
}

static bool if_ray_cross_anedge(const XY& p, const point& kn /*normalized direction vector*/, const IEDGE& e)
{
  point ortho(-kn.y, kn.x);
  double y_loc1 = (point(e.p1->x, e.p1->y) - point(p.x, p.y)) * kn;
  double y_loc2 = (point(e.p2->x, e.p2->y) - point(p.x, p.y)) * kn;
  
  if(y_loc1 < 0 && y_loc2 < 0)
    return false;

  double x_loc1 = (point(e.p1->x, e.p1->y) - point(p.x, p.y)) * ortho;
  double x_loc2 = (point(e.p2->x, e.p2->y) - point(p.x, p.y)) * ortho;

  if(x_loc1 * x_loc2 >= 0)
    return false;

  if(y_loc1 > 0 && y_loc2 > 0)
    return true;

  //double y = (x_loc2 * y_loc1 - x_loc1 * y_loc2) / (x_loc2 - x_loc1);
  if((x_loc2 * y_loc1 - x_loc1 * y_loc2) * (x_loc2 - x_loc1) <= 0)
    return false;

  return true;
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



bool POLY::is_inside_simple(XY& p) const
{
  std::vector<IEDGE*> edges2test;
  point kn(0.0, 1.0);
  //we have to test all edges since tree is now built
  for(auto it : this->edges)
    edges2test.emplace_back(it);
  //now all edges which should be tested with line is in edges2test
  int count(0);
  for(auto e : edges2test)
  {
    if(if_ray_cross_anedge(p,kn,*e))
      count++;
  }
  //std::cout << count << std::endl;
  if(count % 2 == 0)
    return false;
  else
    return true;  
}



bool POLY::is_inside(XY& p/*, FILE* fptr*/) const
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

  std::set<IEDGE*> edges2test;
  //select crossing line
  std::vector<node<IEDGE>*> leaves;
  traverse_edgetree_with_line(&(this->edgetree.master), &leaves, p, kn);
  
  //now all leafnodes crossing line is in leaves
  for(auto nd : leaves)
  {
    for(auto ed_ptr : nd->items)
      edges2test.insert(ed_ptr);
  }

  //now all edges which should be tested with line is in edges2test
  int count(0);
  for(auto e : edges2test)
  {
    if(if_ray_cross_anedge(p,kn,*e))
      count++;
  }
  //std::cout << count << std::endl;
  if(count % 2 == 0)
    return false;
  else
    return true;
  
  /*
  bool res;
  if(count % 2 == 0)
    res=false;
  else
    res=true;

  if(fptr)
  {
    if(res==is_inside_simple(p))
      return res;

    fprintf(fptr,"color_outline 1.0 0.0 1.0 \n");
    fprintf(fptr,"color_no_fill \n");
    fprintf(fptr,"outline_width 0.3 \n");
    for(auto n : leaves)
    {
      fprintf(fptr, "box_fill %g %g %g %g\n", n->node_bounds.pmin.x, n->node_bounds.pmin.y, n->node_bounds.pmax.x, n->node_bounds.pmax.y);
    }
    double index=0;
    for(auto e : edges2test)
    {
      fprintf(fptr,"color_outline %g 0.0 %g \n", index, 1.0-index);
      index += 1.0/double(edges2test.size()-1);
      fprintf(fptr,"outline_width %g \n", index/0.5);
      fprintf(fptr, "e %g %g %g %g\n", e->p1->x, e->p1->y, e->p2->x, e->p2->y);
      printf("%s\n",if_ray_cross_anedge(p,kn,*e)?"cross":"no cross");
    }
    printf("number of edges = %d, res=%s\n",edges2test.size(),res?"true":"false");
  }
  return res;
  */
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
  get_bounds(e,eb);
  if(!eb.cross(b))
    return false;
  double y_xmin = (e.p1->y * e.p2->x - e.p2->y * e.p1->x + b.pmin.x * (e.p2->y - e.p1->y)) / (e.p2->x - e.p1->x);
  double y_xmax = (e.p1->y * e.p2->x - e.p2->y * e.p1->x + b.pmax.x * (e.p2->y - e.p1->y)) / (e.p2->x - e.p1->x);
  double x_ymin = (e.p1->x * e.p2->y - e.p2->x * e.p1->y + b.pmin.y * (e.p2->x - e.p1->x)) / (e.p2->y - e.p1->y);
  double x_ymax = (e.p1->x * e.p2->y - e.p2->x * e.p1->y + b.pmax.y * (e.p2->x - e.p1->x)) / (e.p2->y - e.p1->y);
  if( (y_xmin - b.pmin.y) * (y_xmin - b.pmax.y) <= 0 ||
      (y_xmax - b.pmin.y) * (y_xmax - b.pmax.y) <= 0 ||
      (x_ymax - b.pmin.x) * (x_ymax - b.pmax.x) <= 0 ||
      (x_ymax - b.pmin.x) * (x_ymax - b.pmax.x) <= 0 )
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
