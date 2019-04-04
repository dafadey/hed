#pragma once
#include <vector>
#include <array>
#include <limits>
#include <stdio.h>
#include "primitives.h"

template <typename T>
bool is_inside(const T&, const bounds&); // user must define this function somewhere

template <typename T>
void get_bounds(const T&, bounds&); // user must define this function somewhere

template <class T>
struct node
{
  bool is_leaf;
  bounds node_bounds;
  std::array<node*,4> childs;
  std::vector<T*> items;
  void fill_node(std::vector<T*> items);
};

template <class T>
struct spatial_tree
{
  spatial_tree() : is_filled(false)
  {
    master.is_leaf = false;
  }
  bool is_filled;
  node<T> master;
  void fill_tree(std::vector<T*> items);
  node<T>* get_node(double x, double y);
};

template <class T>
void node<T>::fill_node(std::vector<T*> items)
{
  //static int level(0);
  //level++;
  //just create bounds
  // ^ y
  // |
  // |
  // |
  // B----------------pmax
  // |       |        |
  // |   3   |    2   |
  // |       |        |
  // ------------------
  // |       |        |
  // |   0   |    1   |
  // |       |        |
  // pmin-------------A------->
  //                          x
  std::array<bounds,4> bnds;
  const bounds* nb = &(this->node_bounds);
  bnds[0].pmin = nb->pmin;
  bnds[0].pmax = (nb->pmax + nb->pmin) * 0.5;
  bnds[2].pmin = bnds[0].pmax;
  bnds[2].pmax = nb->pmax;
  bnds[1].pmin = point(bnds[0].pmax.x, nb->pmin.y);
  bnds[1].pmax = point(nb->pmax.x, bnds[0].pmax.y);
  bnds[3].pmin = point(nb->pmin.x, bnds[0].pmax.y);
  bnds[3].pmax = point(bnds[0].pmax.x, nb->pmax.y);

  //printf("center is (%g,%g)\n",bnds[0].pmax.x,bnds[0].pmax.y);

  //try split
  std::array<std::vector<T*>, 4> child_items;
  int all_child_items_count(0);
  for(auto item : items)
  {
    for(int i = 0; i != bnds.size(); i++)
      if(is_inside(*item, bnds[i]))
      {
        child_items[i].push_back(item);
        all_child_items_count++;
      }
  }
  //for(int i = 0; i != bnds.size(); i++)
  //  printf("child %d has %d items\n",i,child_items[i].size());
  //printf("all child items count is %d, items size is %d\n",all_child_items_count, items.size());
  //analyze
  if(all_child_items_count >= 2 * items.size() || items.size() <= 2) // 2 is empirical change it if needed
  {
    //ok this is leaf node. mark and push all items here
    this->is_leaf = true;
    for(auto item : items)
      this->items.push_back(item);
    //printf("closing branch\n");
    //level--;
    return;
  }
  else // create new nodes and call fill_node recursivelly
  {
    //printf("diving one level down now at %d\n",level);
    //create new nodes
    for(int i = 0; i != bnds.size(); i++)
    {
      this->childs[i] = new node<T>;
      this->childs[i]->node_bounds = bnds[i];
      this->childs[i]->is_leaf = false;
      this->childs[i]->fill_node(child_items[i]);
    }
  }
}

template <class T>
void spatial_tree<T>::fill_tree(std::vector<T*> items)
{
  printf("hello spatial_tree::fill_tree is invoked\n");
  point* pmin = &(this->master.node_bounds.pmin);
  point* pmax = &(this->master.node_bounds.pmax);
  pmin->x=std::numeric_limits<double>::max();
  pmax->x=-pmin->x;
  pmin->y=pmin->x;
  pmax->y=-pmin->y;
  for(auto item : items)
  {
    bounds bnd;
    get_bounds(*item, bnd);
    
    pmin->x = std::min(bnd.pmin.x, pmin->x);
    pmin->y = std::min(bnd.pmin.y, pmin->y);
    pmax->x = std::max(bnd.pmax.x, pmax->x);
    pmax->y = std::max(bnd.pmax.y, pmax->y);
  }
  printf("master bounding box is (%g,%g)--(%g,%g)\n",pmin->x,pmin->y,pmax->x,pmax->y);
  this->master.fill_node(items);
  this->is_filled = true;
}
