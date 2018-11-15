#include "resonators.h"
#include <iostream>

structured_resonator::structured_resonator() : resonator_base(), nx(0), ny(0), lx(.0), ly(.0) {}

structured_resonator::structured_resonator(int _nx, int _ny, double _lx, double _ly) : resonator_base(), nx(_nx), ny(_ny), lx(_lx), ly(_ly)
{
  B.resize(nx*ny);
  E.resize((nx+1)*ny + (ny+1)*nx); //say Ey goes first ny*(nx+1) entries, then goes Ex - (ny+1)*nx entries
}

double structured_resonator::step(const cplx& dt)
{
  double dx=lx/(double) nx;
  double dy=ly/(double) ny;
  double dxdy = dx*dy;

  //first do Ey
  for(int i(0); i != (nx+1)*ny; i++)
  {
    int ix = i % (nx+1);
    if(ix==0 || ix == nx)
      continue;
    int iy = i / (nx+1);
    E[i] += (B[ix-1+iy*nx] - B[ix+iy*nx]) / dx * dt;
  }
  //now Ex
  for(int _i((nx+1)*ny); _i != E.size(); _i++)
  {
    int i=_i-(nx+1)*ny;
    int iy = i / nx;
    if(iy==0 || iy == ny)
      continue;
    int ix = i % nx;
    E[_i] += (B[ix+iy*nx] - B[ix+(iy-1)*nx]) / dy * dt;
  }
  
  //zero
  for(int i=0; i!=nx; i++)
  {
    E[(nx+1)*ny+i]=cplx(.0, .0);
    E[(nx+1)*ny+i+ny*nx]=cplx(.0, .0);
  }
  for(int i=0; i!=ny; i++)
  {
    E[i*(nx+1)]=cplx(.0, .0);
    E[i*(nx+1)+nx]=cplx(.0, .0);
  }
  
  for(int i(0); i != B.size(); i++)
  {
    int ix = i % nx;
    int iy = i / nx;
    const cplx* Ey = &E.data()[0];
    const cplx* Ex = &E.data()[(nx+1)*ny];
    B[i] += (
            (Ey[ix+iy*(nx+1)] - Ey[ix+1+iy*(nx+1)]) * dy + 
            (-Ex[ix+iy*nx] + Ex[ix+(iy+1)*nx]) * dx
            ) / dxdy * dt;
  }
  
  return norm() / dt.imag();
}

bool structured_resonator::is_same(const structured_resonator& r) const
{
  if(nx == r.nx && ny == r.ny && lx == r.lx && ly == r.ly)
    return true;
  return false;
}
 
void structured_resonator::withdraw(const resonator_base* r_ptr)
{
  const structured_resonator* sr_ptr = dynamic_cast<const structured_resonator*>(r_ptr);
  if(!sr_ptr)
  {
    std::cerr << "ERROR : you can withdraw only structured resonator from structured resonator\n";
    assert(false);
  }
  const structured_resonator& r = *sr_ptr;
  
  if(!is_same(r))
  {
    std::cerr << "ERROR : you can withdraw resonators with different geometry\n";
    assert(false);
  }
  
  double dx=lx/(double) nx;
  double dy=ly/(double) ny;
  double dxdy = dx*dy;
  cplx k(.0, .0);
  for(int i(1); i!= E.size()-1; i++)
    k += E[i] * r.E[i] * dxdy;
  for(int i(0); i!= B.size(); i++)
    k -= B[i] * r.B[i] * dxdy;
  
  cplx n(.0, .0);
  for(int i(1); i!= E.size()-1; i++)
    n += r.E[i] * r.E[i] * dxdy;
  for(int i(0); i!= B.size(); i++)
    n -= r.B[i] * r.B[i] * dxdy;
  
  k /= n;
  for(int i(0); i!= E.size(); i++)
    E[i] -= k * r.E[i];
  for(int i(0); i!= B.size(); i++)
    B[i] -= k * r.B[i];
}  
  
structured_resonator& structured_resonator::operator = (const structured_resonator& r)
{
  nx=r.nx;
  ny=r.ny;
  lx=r.lx;
  ly=r.ly;
  
  copy_fields(&r);
    
  return *this;
}

