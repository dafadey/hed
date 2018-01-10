#pragma once
#include <vector>
#include <array>
#include <iostream>

struct ITRIANGLE;
struct IEDGE;

struct XY
{
  XY() {}
  XY(double _x, double _y) : x(_x), y(_y), tris(), edges() {}
  double x,y;
  std::vector<ITRIANGLE*> tris;
  std::vector<IEDGE*> edges;
  size_t id;
  XY operator+(const XY&);
  XY operator-(const XY&);
  XY operator*(const double&);
  void operator=(const XY&);
  bool operator==(const XY&) const;
};
XY operator+(const XY& p1, const XY& p2);
XY operator-(const XY& p1, const XY& p2);
XY operator*(const XY& p1, const double& a);
XY operator*(const double& a, const XY& p1);
double vp(const XY& p0, const XY& p1); // vector product
double dp(const XY& p0, const XY& p1); // dot product
std::ostream& operator<<(std::ostream&, const XY&);

XY* newXY(double _x, double _y);

struct ITRIANGLE
{
  ITRIANGLE() {}
  ITRIANGLE(const XY* _p1, const XY* _p2, const XY* _p3) : p1((XY*) _p1), p2((XY*) _p2), p3((XY*) _p3), /*nbrs(),*/ edges() {}
  XY* p1;
  XY* p2;
  XY* p3;
  size_t id;
  //std::array<ITRIANGLE*,3> nbrs;
  std::array<IEDGE*,3> edges;
  XY* point(int /*point index 0, 1 or 2*/); //accesor
  XY centroid();
  double area();
};

std::ostream&  operator<<(std::ostream&, const ITRIANGLE&);

ITRIANGLE* newITRIANGLE(XY* _p1, XY* _p2, XY* _p3);

struct IEDGE
{
  IEDGE() {}
  IEDGE(const XY* _p1, const XY* _p2) : p1((XY*)_p1), p2((XY*)_p2), t1(nullptr), t2(nullptr) {}
  XY* p1;
  XY* p2;
  size_t id;
  //IEDGE* nb1;
  //IEDGE* nb2;
  ITRIANGLE* t1;
  ITRIANGLE* t2;
  XY centroid();
};

std::ostream&  operator<<(std::ostream&, const IEDGE&);

IEDGE* newIEDGE(XY* _p1, XY* _p2);
