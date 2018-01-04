#include "vectors.h"
#include "cmath"

// MEMBERS

vec_type vec_type::operator+(const vec_type& v) const
{
  vec_type res(*this);
  res[0] += v[0];
  res[1] += v[1];
  return res;
}

vec_type vec_type::operator-(const vec_type& v) const
{
  vec_type res(*this);
  res[0] -= v[0];
  res[1] -= v[1];
  return res;
}

vec_type vec_type::operator*(double a) const
{
  vec_type res(*this);
  res[0] *= a;
  res[1] *= a;
  return res;
}

vec_type vec_type::operator/(double a) const
{
  vec_type res(*this);
  res[0] /= a;
  res[1] /= a;
  return res;
}

double vec_type::norm2() const
{
  return (*this)[0] * (*this)[0] + (*this)[1] * (*this)[1];
}

double vec_type::norm() const
{
  return sqrt((*this)[0] * (*this)[0] + (*this)[1] * (*this)[1]);
}

// NON MEMBERS

vec_type operator+(const vec_type& v1, const vec_type& v2)
{
  return vec_type (v1[0] + v2[0], v1[1] + v2[1]);
}

vec_type operator-(const vec_type& v1, const vec_type& v2)
{
  return vec_type (v1[0] - v2[0], v1[1] - v2[1]);
}

vec_type operator*(double a, const vec_type& v)
{
  vec_type(a * v[0], a * v[1]);
}

double vp(const vec_type& p0, const vec_type& p1)
{
	return p0[0] * p1[1] - p0[1] * p1[0];
}
double dp(const vec_type& p0, const vec_type& p1)
{
	return p0[0] * p1[0] + p0[1] * p1[1];
}
