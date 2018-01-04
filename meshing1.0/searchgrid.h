/* this is a structure generally speaking implementing a concept of octree
 * the difference is that it is easier and intended to work with more regular patterns
*/
#pragma once
#include "primitives.h"
#include <vector>
#include <cmath>
#include <fstream>
#include <iostream>

template <class T>
struct search_grid
{
  search_grid() : OPC(64), nx(0), ny(0), dx_1(.0), dy_1(.0), b(), data() {}
	int OPC; // objects per cell
	int nx, ny;
	double dx_1, dy_1;
	bounds b;
	std::vector<std::vector<const T*>> data;
	void add_one(const T*);
	void add(std::vector<T*>&);
	void fill(std::vector<T*>&);
	void initialize_from_data(std::vector<T*>&);
	void clear();
	//subclassing functions to avoid problems with mangling (do not trust namesapces so...)
	void get_bounds(const T&, bounds&); // user must define this function somewhere
  const std::vector<const T*>& get_cell(double x, double y) const;
};

template <typename T>
std::ostream& operator<<(std::ostream& s, const search_grid<T>& sg)
{
  std::cout << "dumping search grid (" << sg.ny << "," << sg.nx << ")\n";
  for(int j=0;j<sg.ny;j++)
  {
    for(int i=0;i<sg.nx;i++)
    {
      s << "r " << i+j*sg.nx << "\n";
      s << "box " << sg.b.pmin.x + double(i) / sg.dx_1 << " "
                 << sg.b.pmin.y + double(j) / sg.dy_1 << " "
                 << sg.b.pmin.x + double(i + 1) / sg.dx_1 << " "
                 << sg.b.pmin.y + double(j + 1) / sg.dy_1 << "\n";
      for(auto it : sg.data[i+j*sg.nx])
        s << *it;
    }
  }
  return s;
}

template <typename T>
void search_grid<T>::clear()
{
	for(auto& it : data)
		it.clear();
}

template <typename T>
void search_grid<T>::initialize_from_data(std::vector<T*>& input)
{
  for(const auto& it : input)
  {
    bounds itb;
    get_bounds(*it, itb);
    b.pmin.x = std::min(itb.pmin.x, b.pmin.x);
    b.pmax.x = std::max(itb.pmax.x, b.pmax.x);
    b.pmin.y = std::min(itb.pmin.y, b.pmin.y);
    b.pmax.y = std::max(itb.pmax.y, b.pmax.y);
  }
  
  const double d = sqrt((b.pmax.x - b.pmin.x) * (b.pmax.y - b.pmin.y) / (double) input.size() * (double) OPC);
  std::cout << "d = " << d << "\n";
  if(OPC!=-1)
  {
    nx = ceil((b.pmax.x - b.pmin.x) / d);
    ny = ceil((b.pmax.y - b.pmin.y) / d);
  }
  data.resize(nx * ny);
  dx_1 = (double) nx / (b.pmax.x - b.pmin.x);
  dy_1 = (double) ny / (b.pmax.y - b.pmin.y);
  std::cout << "X = " << 1.0/dx_1 << "\n";
  std::cout << "Y = " << 1.0/dy_1 << "\n";
}

template <typename T>
void search_grid<T>::add_one(const T* item)
{
  bounds bb;
  this->get_bounds(*item, bb);
  int imin = floor(bb.pmin.x - b.pmin.x) * dx_1;
  int imax = ceil(bb.pmax.x - b.pmin.x) * dx_1;
  int jmin = floor(bb.pmin.y - b.pmin.y) * dy_1;
  int jmax = ceil(bb.pmax.y - b.pmin.y) * dy_1;
  imin = imin < 0 ? 0 : imin;
  jmin = jmin < 0 ? 0 : jmin;
  imax = imax >= nx ? nx - 1 : imax;
  jmax = jmax >= ny ? ny - 1 : jmax;
  if(imax < 0 || imin >= nx || jmax < 0 || jmin >= ny)
    std::cerr << "consider rebuilding search grid(" << nx << ", " << ny
              << "), you requested cell with (" << imin << ", " << jmin << ")-(" << imax << ", " << jmax << ")" << "\n";

  for(int i(imin); i != imax + 1; i++)
  {
    for(int j(jmin); j != jmax + 1; j++)
    {
      bool have(false);
      for(auto it : data[i + j * nx])
      {
        if(it == item)
        {
          have = true;
          break;
        }
      }
      if(!have)
        data[i + j * nx].push_back(item);
    }
  }
}

template <typename T>
const std::vector<const T*>& search_grid<T>::get_cell(double x, double y) const
{
  int i = floor((x - b.pmin.x) * dx_1);
  int j = floor((y - b.pmin.y) * dy_1);
  i = i < 0 ? 0 : i;
  i = i >= nx ? nx - 1 : i;
  j = j < 0 ? 0 : j;
  j = j >= ny ? ny - 1 : j;
//  std::cout << "get_cell : (" << y << ", " << x << ") -> (" << j << ", " << i << ") size=" << data[i + j * nx].size() << "\n";
//  for(auto p : data[i + j * nx])
//    std::cout << "     (" << p->x << ", " << p->y << ")\n";
  return data[i + j * nx];
}

template <typename T>
void search_grid<T>::add(std::vector<T*>& input)
{
  for(auto it : input)
    add_one((const T*) it);
}

template <typename T>
void search_grid<T>::fill(std::vector<T*>& input)
{
  if(dx_1 == .0 || dy_1 == .0)
    initialize_from_data(input);
  add(input);
}
