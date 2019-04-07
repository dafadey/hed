#include "svg.h"
#include "primitives.h"
#include "geometry.h"
#include "simpledraw.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cmath>

/*
 *    n_____Ex_>______n______Ex_>______n_____Ex_>______ n               
 *    |               |                |               |                
 *    |               |                |               |                
 *    Ey     B        Ey      B        Ey     B        Ey               
 *    |               |                |               |                
 *    |               |                |               |                
 *   n|_____Ex_>______|______Ex_>______|_____Ex_>______|n               
 *    |               |n               |n              |                
 *    |               |                |               |                
 *    Ey             Ey                Ey              |                
 *    |      B        |      B         |       B       Ey   dy          
 *    |               |                |               |                
 *    |_____Ex_>______|______Ex_>______|______Ex_>_____|                
 *   n                n                n                n               
 *                                            dx                        
 */

//#define DRAW

#define DENSITY 0.3

int main(int argc, char* argv[])
{
  int nx=75;
  int ny=81;
  #ifdef DRAW
  fadey_init(nx,ny,4);
  #endif
  if(argc==3)
  {
    nx = atoi(argv[1]);
    ny = atoi(argv[2]);
  }

  std::vector<std::vector<point>> contours;
  svg::importall("input_geo.svg", contours, 0.1);
  std::ofstream of;
  
  for(const auto& c : contours)
  {
    std::cout << c.size() << " :";
    for(const auto& p : c)
      std::cout << " (" << p.x << ", " << p.y << ")";
    std::cout << '\n';
  }
  
  POLY pol;
  for(const auto& pt : contours[1])
    pol.points.emplace_back(newXY(pt.x, pt.y));
  pol.fill_edges();
  pol.fill_tree();
  
  
  double xmin = std::numeric_limits<double>::max();
  double ymin = xmin;
  double xmax = -xmin;
  double ymax = -xmin;
  for(const auto& p : contours[0])
  {
    xmin = std::min(xmin, p.x);
    xmax = std::max(xmax, p.x);
    ymin = std::min(ymin, p.y);
    ymax = std::max(ymax, p.y);
  }
  
  double dx = (xmax - xmin) / (double) nx;
  double dy = (ymax - ymin) / (double) ny;
  
  double* n = new double[(nx + 1) * (ny + 1)];
 
  for(int j=0; j != ny+1; j++)
  {
    for(int i=0; i != nx+1; i++)
    {
      const double x = xmin + dx * (double) i;
      const double y = ymin + dy * (double) j;
      XY xy(x,y);
      n[(nx+1)*j + i] = pol.is_inside(xy) ? DENSITY : 0.0;
    }
  }
  double* bz = new double[nx * ny];
  for(int i=0; i != nx * ny; i++)
  {
    //double x = dx * (double) (i % nx) - (xmax-xmin)*0.25;
    //double y = dy * (double) (i / ny) - (ymax-ymin)*0.25;
    bz[i] = 0.0;//exp(-(x * x + y * y) * 0.1);
  }
  double* ex = new double[nx * (ny + 1)];
  double* jx = new double[nx * (ny + 1)];
  for(int i=0; i != nx * (ny + 1); i++)
  {
    ex[i] = 0.0;
    jx[i] = 0.0;
  }
  double* ey = new double[(nx + 1) * ny];
  double* jy = new double[(nx + 1) * ny];
  for(int i=0; i != (nx + 1) * ny; i++)
  {
    ey[i] = 0.0;
    jy[i] = 0.0;
  }

  std::cout << "dxmin=" << dx << '\n';
  std::cout << "dymin=" << dy << '\n';

  //source will be calculated only for ex
  // get source line coords
  double sxmin = std::min(contours[2][0].x, contours[2][1].x);
  double sxmax = std::max(contours[2][0].x, contours[2][1].x);
  double sy = contours[2][0].y;
  if(contours[2][0].y != contours[2][1].y)
  {
    std::cerr << "source line is not x aligned exiting\n";
    return -1;
  }
  int ismin = (sxmin-xmin) / dx;
  int ismax = (sxmax-xmin) / dx;
  int js = (sy - ymin) / dy;

  #ifdef DRAW
  fadey_draw(bz, nx, ny, 0);
  fadey_draw(n, nx+1, ny+1, 1);
  fadey_draw(ex, nx, ny+1, 2);
  fadey_draw(ey, nx+1, ny, 3);
  char a;
  std::cin >> a;
  #endif
  
  double dt = 0.2;
  double Tmax=200.0;
  double t(0.0);
  int step(0);
  int num_digits_in_filename = ceil(log10(2.0*Tmax/dt));
  while(t < Tmax)
  {
    // add source
    for(int i=ismin; i<ismax; i++)
      ex[js * nx + i] += sin(t*.1) * dt;
    // upd ex, ey
    for(int j=0; j != ny; j++)
    {
      int i=0;
      ey[j * (nx + 1) + i] += (-(bz[j * nx + i] - 0.0) / dx + jy[j * (nx + 1) + i]) * dt;
      i++;
      for(; i != nx; i++)
        ey[j * (nx + 1) + i] += (-(bz[j * nx + i] - bz[j * nx + i - 1]) / dx + jy[j * (nx + 1) + i]) * dt;
      ey[j * (nx + 1) + i] += (-(0.0 - bz[j * nx + i - 1]) / dx + jy[j * (nx + 1) + i]) * dt;
    
    }
    
    {
      int j=0;
      for(int i=0; i != nx; i++)
        ex[j * nx + i] += ((bz[j * nx + i] - 0.0) / dy + jx[j * nx + i])* dt;
      j++;
      for(; j != ny; j++)
      {
        for(int i=0; i != nx; i++)
          ex[j * nx + i] += ((bz[j * nx + i] - bz[(j - 1) * nx + i]) / dy + jx[j * nx + i])* dt;
      }
      for(int i=0; i != nx; i++)
        ex[j * nx + i] += ((0.0 - bz[(j - 1) * nx + i]) / dy + jx[j * nx + i])* dt;
    }
    
    // upd bz
    for(int j=0; j != ny; j++)
    {
      for(int i=0; i != nx; i++)
        bz[j * nx + i] += (-(ex[j * nx + i] - ex[(j + 1) * nx + i]) / dy +
                          (ey[j * (nx + 1) + i] - ey[j * (nx + 1) + i + 1]) / dx) * dt;
    }
    
    // upd jx, jy
    for(int j=0; j != ny; j++)
    {
      for(int i=0; i != nx + 1; i++)
      {
        double _n = 0.5 * (n[j * (nx + 1) + i] + n[(j + 1) * (nx + 1) + i]);
        if(n[j * (nx + 1) + i] == 0.0 || n[(j + 1) * (nx + 1) + i] == 0.0)
          _n = 0.0;
        jy[j * (nx + 1) + i] += -_n * ey[j * (nx + 1) + i] * dt;
      }
    }
    for(int j=0; j != ny + 1; j++)
    {
      for(int i=0; i != nx; i++)
      {
        double _n = 0.5 * (n[j * (nx + 1) + i] + n[j * (nx + 1) + i  + 1]);
        if(n[j * (nx + 1) + i] == 0.0 || n[j * (nx + 1) + i  + 1] == 0.0)
          _n = 0.0;
        jx[j * nx + i] += -_n * ex[j * nx + i] * dt;
      }
    }


    if(step % 10 == 0)
    {
      std::stringstream filename;
      filename << "fields/yee_fields" << std::setw(num_digits_in_filename) << std::setfill('0') << step;
      of.open(filename.str().c_str());
      
      for(int j=0; j != ny+1; j++)
      {
        for(int i=0; i != nx+1; i++)
          of << "p " << xmin+double(i)*dx << " " << ymin+double(j)*dy << " "
                     << n[(nx + 1) * j + i] << '\n';
      }
      for(int j=0; j != ny+1; j++)
      {
        for(int i=0; i != nx; i++)
          of << "e " << xmin+double(i)*dx << " " << ymin+double(j)*dy << " "
                     << xmin+double(i+1)*dx << " " << ymin+double(j)*dy << " "
                     << ex[nx * j + i] << '\n';
      }
      for(int j=0; j != ny; j++)
      {
        for(int i=0; i != nx+1; i++)
          of << "e " << xmin+double(i)*dx << " " << ymin+double(j)*dy << " "
                     << xmin+double(i)*dx << " " << ymin+double(j+1)*dy << " "
                     << ey[(nx + 1) * j + i] << '\n';
      }
      for(int j=0; j != ny; j++)
      {
        for(int i=0; i != nx; i++)
          of << "box " << xmin+double(i)*dx << " " << ymin+double(j)*dy << " "
                       << xmin+double(i+1)*dx << " " << ymin+double(j+1)*dy << " "
                       << bz[nx * j + i] << '\n';
      }
      of.close();

    }
    
    // update t
    t += dt;
    step++;
    
    #ifdef DRAW
    fadey_draw(bz, nx, ny, 0);
    fadey_draw(n, nx+1, ny+1, 1);
    fadey_draw(jx, nx, ny+1, 2);
    fadey_draw(jy, nx+1, ny, 3);
    #endif

  }

  #ifdef DRAW
  fadey_draw(bz, nx, ny, 0);
  fadey_draw(n, nx+1, ny+1, 1);
  fadey_draw(ex, nx, ny+1, 2);
  fadey_draw(ey, nx+1, ny, 3);
  std::cin >> a;
  fadey_close();
  #endif

  delete[] n;
  delete[] bz;
  delete[] ex;
  delete[] jx;
  delete[] ey;
  delete[] jy;
  return 0;
}
