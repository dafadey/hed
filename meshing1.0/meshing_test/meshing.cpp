#include "../triangulate.h"
#include <stdio.h>
#include <cstdlib>
#include <vector>
#include <limits>
#include <cmath>
#include <algorithm>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

struct cells
{
  cells(int _nx=10, int _ny=10) : nx(_nx), ny(_ny), xmin(std::numeric_limits<double>::max()), ymin(std::numeric_limits<double>::max()), xmax(-std::numeric_limits<double>::max()), ymax(-std::numeric_limits<double>::max()), data()
  {
    data = new std::vector<XY*>[nx*ny];
  }
  ~cells()
  {
    delete[] data;
  }
  int nx, ny;
  double dx,dy;
  double xmin,xmax,ymin,ymax;
  std::vector<XY*>* data;

  //methods
  std::vector<XY*>* cell(int i, int j)
  {
    if(i>=nx || j>=ny || i<0 || j<0)
      return nullptr;
    return &data[j * nx + i];
  }
  
  void get_cell_ids(double x, double y, int& i, int &j)
  {
    i = (x == xmax) ? nx-1 : (int) floor((x - xmin) / dx);
    j = (y == ymax) ? ny-1 : (int) floor((y - ymin) / dy);
  }
  
  std::vector<XY*>* cell(double x, double y)
  {
    int i,j;
    get_cell_ids(x, y, i, j);
    //printf("cell [%d,%d] found for point (%g,%g)\n", i, j, x, y);
    return cell(i, j);
  }
  
  bool push_point(XY* pt)
  {
    auto c = cell(pt->x, pt->y);
    //printf("adding point (%g,%g) to cell %p with size %d\n", pt->x, pt->y, c, c->size());
    if(!c)
      return false;
    c->push_back(pt);
    return true;
  }
  
  void getBB(std::vector<XY*>& pts)
  {
    for(int i=0;i<pts.size();i++)
    {
      //printf("analysing point %g,%g\n",pts[i].x,pts[i].y);
      xmin = std::min(pts[i]->x, xmin);
      ymin = std::min(pts[i]->y, ymin);
      xmax = std::max(pts[i]->x, xmax);
      ymax = std::max(pts[i]->y, ymax);
    }
    dx = (xmax - xmin) / (double) nx;
    dy = (ymax - ymin) / (double) ny;
    //printf("cell:: getBB: xmin=%g, ymin=%g, xmax=%g, ymax=%g\n",xmin,ymin,xmax,ymax);
  }
    
  void fill(std::vector<XY*>& pts)
  {
    for(int i = 0; i < pts.size(); i++)
      push_point(pts[i]);
  }
  void clear()
  {
    for(int i=0;i<nx*ny;i++)
      data[i].clear();
  }
};

double distance(XY* p1, XY* p2)
{
  return sqrt((p1->x-p2->x)*(p1->x-p2->x)+(p1->y-p2->y)*(p1->y-p2->y));
}

std::vector<XY*> get_closest_points(XY* p, cells& cs, int n=4)
{
  std::vector<std::pair<XY*, double>> neibs;
  const int incs[9][2]={{0,0},{1,0},{-1,0},{0,1},{0,-1},{1,1},{-1,1},{1,-1},{-1,-1}};  
  int i,j;
  cs.get_cell_ids(p->x, p->y, i, j);
  for(int k = 0; k != 9; k++)
  {
    int ii = i + incs[k][0];
    int jj = j + incs[k][1];
    auto c = cs.cell(ii,jj);
    if(c)
    {
      for(int q=0;q!=c->size();q++)
      {
        if((*c)[q] != p)
          neibs.emplace_back((*c)[q], distance(p,(*c)[q]));
      }
    }
  }
  std::sort(neibs.begin(),neibs.end(), [](const std::pair<XY*,double> & a, const std::pair<XY*,double> & b) -> bool {return a.second < b.second;});
  std::vector<XY*> res;
  for(auto item : neibs)
  {
    res.emplace_back(item.first);
    if(res.size()==n) 
      break;
  }
  //printf("returning %d closest points to %g,%g\n", res.size(), p->x, p->y);
  return res;
}

int main(int argc,char **argv)
{  
  std::vector<ITRIANGLE*> v;
  std::vector<XY*> p;
  int nv = 0;
   
  nv=1000;
  FILE *fptr;
  //FILE *fptr = fopen("geom.dat","w");
  for(int i=0;i!=nv;i++)
  {
    XY* pt = new XY;
    pt->x = (double) rand() / (double) RAND_MAX;
    pt->y = (double) rand() / (double) RAND_MAX;
    p.push_back(pt);
    //fprintf(fptr,"p %g %g\n",p[i].x,p[i].y);
  }
  //fclose(fptr);

  cells c(10,10);
  c.getBB(p);
  double target_l=sqrt(2.0*(c.xmax-c.xmin)*(c.ymax-c.ymin)/double(nv)/sqrt(3.0));
  printf("target_l=%g\n",target_l);
  for(int iter=0;iter!=57;iter++)
  {
    std::stringstream ss;
    ss << "geom_" << std::setw(3) << std::setfill('0') << iter << std::setfill(' ') << ".dat";
    std::string filename=ss.str();
    c.fill(p);
    //printf("cells fill is done\n");
    std::vector<XY> vels;
    for(int ptid = 0; ptid != p.size(); ptid++)
    {
      XY vel;
      vel.x = 0.0;
      vel.y = 0.0;
      int Nnb=1;
      for(auto nb : get_closest_points(p[ptid], c, Nnb))
      {
        //printf("doing neib pt at %g,%g\n", nb->x, nb->y);
        double f = (distance(nb,p[ptid]) - target_l) / target_l;
        //f=(f>0)?f/(1.0+f):f;
        //f=(f<0)?-pow(std::abs(f),0.75):pow(std::abs(f),0.75);
        //double f = 0.005/(0.003+distance(*nb,p[ptid]));
        vel.x -= f * (p[ptid]->x - nb->x) / double(Nnb);
        vel.y -= f * (p[ptid]->y - nb->y) / double(Nnb);
      }
      #define BND_REACT 0.005
      if(c.xmax - p[ptid]->x < BND_REACT)
      {
        double f = 0.1/(1.0 + c.xmax - p[ptid]->x);
        vel.x += p[ptid]->x - c.xmax;
      }
      if(p[ptid]->x - c.xmin < BND_REACT)
      {
        double f = 0.1/(1.0 + p[ptid]->x - c.xmin);
        vel.x += c.xmin - p[ptid]->x;
      }
      if(c.ymax - p[ptid]->y < BND_REACT)
      {
        double f = 0.1/(1.0 + c.ymax - p[ptid]->y);
        vel.y += p[ptid]->y - c.ymax;
      }
      if(p[ptid]->y - c.ymin < BND_REACT)
      {
        double f = 0.1/(1.0 + p[ptid]->y - c.ymin);
        vel.y += c.ymin - p[ptid]->y;
      }
      vels.push_back(vel);
    }
    //printf("v calc is done\n");

    //fptr = fopen("geom.dat","a");
    //fprintf(fptr,"new");
    for(int ptid = 0; ptid != p.size(); ptid++)
    {
    
      p[ptid]->x += vels[ptid].x;
      p[ptid]->x = (p[ptid]->x < c.xmin) ? c.xmin : p[ptid]->x;
      p[ptid]->x = (p[ptid]->x > c.xmax) ? c.xmax : p[ptid]->x;
        
      p[ptid]->y += vels[ptid].y;
      p[ptid]->y = (p[ptid]->y < c.ymin) ? c.ymin : p[ptid]->y;
      p[ptid]->y = (p[ptid]->y > c.ymax) ? c.ymax : p[ptid]->y;
      //fprintf(fptr,"p %g %g\n",p[ptid].x,p[ptid].y);
    }
    //fclose(fptr);
    c.clear();
    
    printf("triangulating\n");
    Triangulate(p,v);
    printf("Formed %d triangles\n",v.size());

    /* Write triangles in geom format */
    fptr = fopen(filename.c_str(),"a");
    for (int i=0;i<v.size();i++) {
      fprintf(fptr,"t %g %g %g %g %g %g\n",
      v[i]->p1->x, v[i]->p1->y,
      v[i]->p2->x, v[i]->p2->y,
      v[i]->p3->x, v[i]->p3->y);
    }
    fclose(fptr);    
    
    //run asy
    {
      std::string cmd = "echo \"file f=input(\\\"" + filename + "\\\").word();\" > " + filename + ".asy";
      printf("running : %s\n",cmd.c_str());
      std::system(cmd.c_str());
      //target_l
      std::stringstream ss;
      ss << "echo  \"real s0=" << target_l << ";\" >> " << filename << ".asy";
      cmd = ss.str();
      printf("running : %s\n",cmd.c_str());
      std::system(cmd.c_str());
      cmd = "cat plot.asy >> " + filename + ".asy";
      printf("running : %s\n",cmd.c_str());
      std::system(cmd.c_str());
      cmd = "asy " + filename + ".asy";
      printf("running : %s\n",cmd.c_str());
      std::system(cmd.c_str());
	    cmd = "rm " + filename + ".asy";
      printf("running : %s\n",cmd.c_str());
      std::system(cmd.c_str());
      cmd = "convert -density 16 " + filename + ".eps -background white -flatten " + filename + ".png";
      printf("running : %s\n",cmd.c_str());
      std::system(cmd.c_str());
      cmd = "rm -f " + filename + ".eps";
      printf("running : %s\n",cmd.c_str());
      std::system(cmd.c_str());
      cmd = "rm " + filename;
      printf("running : %s\n",cmd.c_str());
      std::system(cmd.c_str());
    }
  }
  

  return 0;
}



