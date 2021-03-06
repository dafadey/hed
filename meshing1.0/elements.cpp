#include "elements.h"
#include <vector>


XY* newXY(double _x, double _y)
{
  return new XY(_x, _y);
}

ITRIANGLE* newITRIANGLE(XY* _p1, XY* _p2, XY* _p3)
{
  ITRIANGLE* newt = new ITRIANGLE(_p1, _p2, _p3);
  _p1->tris.push_back(newt);
  _p2->tris.push_back(newt);
  _p3->tris.push_back(newt);
  return newt;
}

IEDGE* newIEDGE(XY* _p1, XY* _p2)
{
  IEDGE* newe = new IEDGE(_p1, _p2);
  _p1->edges.push_back(newe);
  _p2->edges.push_back(newe);
  return newe;
}

double vp(const XY& p0, const XY& p1)
{
	return p0.x * p1.y - p0.y * p1.x;
}
double dp(const XY& p0, const XY& p1)
{
	return p0.x * p1.x + p0.y * p1.y;
}

XY operator+(const XY& p1, const XY& p2)
{
	XY res(p1.x + p2.x, p1.y + p2.y);
	return res;
}
XY operator-(const XY& p1, const XY& p2)
{
	XY res(p1.x - p2.x, p1.y - p2.y);
	return res;
}
XY operator*(const XY& p1, const double& a)
{
	XY res(p1.x * a, p1.y * a);
	return res;
}
XY operator*(const double& a, const XY& p1)
{
	XY res(p1.x * a, p1.y * a);
	return res;
}

//XY::XY::XY::XY::XY::XY::XY::XY::XY::XY::XY::

XY XY::operator+(const XY& p1)
{
	XY res(this->x + p1.x, this->y + p1.y);
	return res;
}
XY XY::operator-(const XY& p1)
{
	XY res(this->x - p1.x, this->y - p1.y);
	return res;
}
XY XY::operator*(const double& a)
{
	XY res(this->x * a, this->y * a);
	return res;
}
void XY::operator=(const XY& p1)
{
	x = p1.x;
	y = p1.y;
}

bool XY::operator==(const XY& p1) const
{
  return x == p1.x && y == p1.y;
}


std::ostream& operator<<(std::ostream& s, const XY& p)
{
  return s << "p " << p.x << " " << p.y;
}

//ITRIANGLE::ITRIANGLE::ITRIANGLE::ITRIANGLE::ITRIANGLE::

XY* ITRIANGLE::point(int i)
{
	switch (i)
	{
		case 0:
			return p1;
			break;
		case 1:
			return p2;
			break;
		case 2:
			return p3;
			break;
		default:
      return nullptr;
			break;
	}
}

XY ITRIANGLE::centroid()
{
	const double a = 1.0/3.0; 
	XY c((p1->x + p2->x + p3->x) * a, (p1->y + p2->y + p3->y) * a);
	return c;
}

double ITRIANGLE::area()
{
	return .5 * vp(*p3 - *p1, *p2 - *p1);
}

bool ITRIANGLE::is_boundary() const
{
  for(auto e : edges)
  {
    if(e->is_boundary())
      return true;
  }
	return false;
}

std::array<ITRIANGLE*, 3> ITRIANGLE::get_neighbors() const
{
  std::array<ITRIANGLE*, 3> output;
  for(size_t e_id(0); e_id != edges.size(); e_id++)
  {
    if(edges[e_id]->t1 == this)
      output[e_id] = edges[e_id]->t2;
    else
      output[e_id] = edges[e_id]->t1;
  }
  return output;
}

std::set<ITRIANGLE*> ITRIANGLE::get_surrounding() const
{
  std::set<ITRIANGLE*> outer_tris;
  //get surroundings
  const std::array<XY*,3> pts{{p1, p2, p3}};
  for(auto pt : pts)
  {
    for(auto tt : pt->tris)
    {
      if(tt != this)
        outer_tris.insert(tt);
    }
  }
  return outer_tris;
}

std::ostream& operator<<(std::ostream& s, const ITRIANGLE& t)
{
  return s << "t " << t.p1->x << " " << t.p1->y
              << " " << t.p2->x << " " << t.p2->y
              << " " << t.p3->x << " " << t.p3->y;
}


//IEDGE::IEDGE::IEDGE::IEDGE::IEDGE::IEDGE::IEDGE::IEDGE::
bool IEDGE::is_boundary() const
{
	return t1 == nullptr || t2 == nullptr;
}

XY IEDGE::centroid()
{
	XY c((p1->x + p2->x) * 0.5, (p1->y + p2->y) * 0.5);
	return c;
}

std::ostream& operator<<(std::ostream& s, const IEDGE& e)
{
  return s << "e " << e.p1->x << " " << e.p1->y
              << " " << e.p2->x << " " << e.p2->y;
}
