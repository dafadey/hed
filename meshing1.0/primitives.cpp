#include "primitives.h"
#include <algorithm>
#include <limits>
#include <cmath>

double point::norm2()
{
  return this->x*this->x + this->y*this->y;
}
double point::norm()
{
  return sqrt(this->norm2());
}

point point::operator+(const point& p1)
{
  point res(this->x + p1.x, this->y + p1.y);
  return res;
}

point point::operator+()
{
  point res(this->x, this->y);
  return res;
}

point point::operator-(const point& p1)
{
  point res(this->x - p1.x, this->y - p1.y);
  return res;
}

point point::operator-()
{
  point res(-this->x, -this->y);
  return res;
}

point point::operator*(const double& a)
{
  point res(this->x * a, this->y * a);
  return res;
}

point point::operator/(const double& a)
{
  point res(this->x / a, this->y / a);
  return res;
}

void point::operator=(const point& p1)
{
  x = p1.x;
  y = p1.y;
}

point operator+(const point& p1, const point& p2)
{
  point res(p2.x + p1.x, p2.y + p1.y);
  return res;
}

point operator-(const point& p1, const point& p2)
{
  point res(p1.x - p2.x, p1.y - p2.y);
  return res;
}

point operator*(const double& a, const point& p)
{
  point res(a * p.x, a * p.y);
  return res;
}

point operator*(const point& p, const double& a)
{
  point res(p.x * a, p.y * a);
  return res;
}

double operator*(const point& p1, const point& p2)
{
  return p1.x * p2.x + p1.y * p2.y;
}

point operator/(const point& p, const double& a)
{
  point res(p.x / a, p.y / a);
  return res;
}




bool bounds::cross(const bounds& b) const
{
  if( b.pmin.x > pmax.x ||
      b.pmax.x < pmin.x ||
      b.pmin.y > pmax.y ||
      b.pmax.y < pmin.y )
    return false;
  return true;
}

bounds::bounds()
{
   pmin = point(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
   pmax = -pmin;
}

bounds::bounds(point& p1, point& p2)
{
  pmin = point(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
  pmin = point(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
}

std::array<point ,4> bounds::generate_box() const
{
  return std::array<point ,4> {{point(pmin.x,pmin.y),
                                point(pmax.x,pmin.y),
                                point(pmax.x,pmax.y),
                                point(pmin.x,pmax.y)}};
}

std::ostream& operator<<(std::ostream& s, const bounds& b)
{
  s << "box " << b.pmin.x << " " << b.pmin.y << " "
              << b.pmax.x << " " << b.pmax.y << "\n";
  return s;
}
