#pragma once
#include <ostream>
#include <array>
#include "elements.h"

struct vec_type : public std::array<double, 2>
{
  vec_type() : std::array<double, 2>{{0,0}} {}
  vec_type(const vec_type& v) : std::array<double, 2>{{v[0], v[1]}} {}
  vec_type(double x, double y) : std::array<double, 2>{{x, y}} {}
  vec_type(const XY* ppt) : std::array<double, 2>{{ppt->x, ppt->y}} {}
  
  
  vec_type operator+(const vec_type& v) const;

  vec_type operator-(const vec_type& v) const;

  vec_type operator*(double a) const;

  vec_type operator/(double a) const;

  double norm2() const;

  double norm() const;
};

double vp(const vec_type& p0, const vec_type& p1);

double dp(const vec_type& p0, const vec_type& p1);

std::ostream& operator<<(std::ostream& s, const vec_type& v);
