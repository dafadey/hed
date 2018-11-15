#pragma once
#include <string>
#include <vector>
#include <complex>
#include <assert.h>
#include <heds/solver.h>

typedef std::complex<double> cplx;

struct resonator_base
{
  resonator_base() : E(), B() {}
  
  std::vector<cplx> E;
  std::vector<cplx> B;
  
  double norm();
  
  void copy_fields(const resonator_base* from);
  
  virtual double step(const cplx&)
  {
    assert(false);
    return .0;
  }
  
  virtual resonator_base& operator = (const resonator_base&)
  {
    assert(false);
    return *this;
  }

  virtual void withdraw(const resonator_base*)
  {
    assert(false);
  }
};

struct structured_resonator : public resonator_base
{
  structured_resonator();
  
  structured_resonator(int _nx, int _ny, double _lx, double _ly);
  
  int nx;
  int ny;
  double lx;
  double ly;
  
  bool is_same(const structured_resonator& r) const;

  virtual double step(const cplx& dt);
  
  virtual void withdraw(const resonator_base* r_ptr);
  
  virtual structured_resonator& operator = (const structured_resonator& r);
  
};

struct unstructured_resonator : public resonator_base
{
  unstructured_resonator();
  
  unstructured_resonator(const std::string& input_geo_file, double seed=7.0);
  
  mesh m;
  weights wgts;
  hed_data hedsol;
  std::vector<size_t> bndry;

  bool is_same(const unstructured_resonator& r) const;
  
  virtual double step(const cplx& dt);
  
  virtual void withdraw(const resonator_base* r_ptr);
 
  virtual unstructured_resonator& operator = (const unstructured_resonator& r);
  
};

std::ostream& operator << (std::ostream& os, const unstructured_resonator& r);
