#pragma once
#include <array>
#include <iostream>

struct point
{
  double x,y;
  point() {}
  point(double _x, double _y) : x(_x), y(_y) {}
  double norm2();
  double norm();
  point operator+(const point&);
  point operator+();
  point operator-(const point&);
  point operator-();
  point operator*(const double&);
  point operator/(const double&);
  void operator=(const point&);
};

point operator+(const point&, const point&);
point operator-(const point&, const point&);
point operator*(const double&, const point&);
point operator*(const point&, const double&);
double operator*(const point&, const point&);
point operator/(const point&, const double&);

struct bounds
{
  bounds();
  bounds(point&, point&);
  bool cross(const bounds&) const; // return true if mutually corss or inside
  point pmin, pmax;
  std::array<point, 4> generate_box() const;
};

std::ostream& operator<<(std::ostream&, const bounds&);

