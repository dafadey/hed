#include "quadtree.h"
#include "geometry.h"
#include "elements.h"
#include <stdio.h>
#include <sstream>
#include <cmath>
#ifndef M_PI
  #define M_PI 3.1415926535897932384626433832795
#endif
#include "timer.h"

void print_leafs(FILE* fptr, node<IEDGE>& n)
{
  if(n.is_leaf)
  {
    if(n.items.size()==0)
    {
      fprintf(fptr,"color_outline 0.5 0.5 0.5 \n");
      fprintf(fptr,"color_no_fill \n");
      fprintf(fptr, "box %g %g %g %g\n", n.node_bounds.pmin.x, n.node_bounds.pmin.y, n.node_bounds.pmax.x, n.node_bounds.pmax.y);
    }
    else
    {
      fprintf(fptr,"color_outline 0.0 0.5 0.0 \n");
      fprintf(fptr,"color_fill 1.0 0.7 0.2 \n");
      fprintf(fptr, "box_fill %g %g %g %g\n", n.node_bounds.pmin.x, n.node_bounds.pmin.y, n.node_bounds.pmax.x, n.node_bounds.pmax.y);
    }
    return;
  }
  for(auto child : n.childs)
    print_leafs(fptr, *child);
}

int main()
{
  POLY p;
  //saw tooth poly:
  double d=0.0125;
  /*
  for(double i=0;i<1.0;i+=d)
  {
    p.points.push_back(newXY(i,0));
    p.points.push_back(newXY(i+d,d));
  }
  p.points.push_back(newXY(1.0,-1.2));
  p.points.push_back(newXY(0,-1.2));
  */

//  for(double a=0.0; a<2.0*M_PI;a+=0.01)
  for(double a=0.0; a<2.0*M_PI;a+=0.01)
  {
    double aa=a/(2.0*M_PI);
    double b=aa*aa*13.0*2.0*M_PI;
    p.points.push_back(newXY((1.0+sin(b)*0.3)*cos(a),(1.0+sin(b)*0.3)*sin(a)));
  }
  p.fill_tree();
  std::string filename="poly.dat";
  FILE *fptr=fopen(filename.c_str(),"w");
  auto master_node = p.edgetree.master;
  print_leafs(fptr, master_node);
  printf("stored leafs\n");
  fprintf(fptr,"color_outline 0.0 0.0 1.0 \n");
  fprintf(fptr,"color_no_fill \n");
  for(auto e : p.edges)
    fprintf(fptr,"e %g %g %g %g\n", e->p1->x, e->p1->y, e->p2->x, e->p2->y);
  printf("stored edges\n");
  {
    std::vector<XY> querys;
//    for(int i=0;i!=10000000;i++)
    for(int i=0;i!=1000000;i++)
    {
      XY pt(4.0 * (double(rand())/double(RAND_MAX) - 0.5), 4.0 * (double(rand())/double(RAND_MAX) - 0.5));
      querys.push_back(pt);
    }
    printf("created query points\n");
    std::vector<XY*> inside;
    {
      timer t("query time is: ");
      for(auto it = querys.begin(); it != querys.end(); it++)
      {
        if(p.is_inside(*it) != p.is_inside_simple(*it))
        {
          inside.push_back(&(*it));
          break;
        }
        if(p.is_inside(*it))
          inside.push_back(&(*it));
      }
    }
    fprintf(fptr,"color_no_outline \n");
    fprintf(fptr,"color_fill 1.0 0.0 0.0 \n");
    for(auto it : inside)
      fprintf(fptr,"p %g %g\n", it->x, it->y);
    printf("found %d problems\n",inside.size());

  }
  fclose(fptr);
  
  
  {
    std::string cmd = "echo file f=input(\"" + filename + "\"); > " + filename + ".asy";
    printf("running : %s\n",cmd.c_str());
    std::system(cmd.c_str());
    //target_l
    std::stringstream ss;
    ss << "echo  real s0=" << 0.1 << "; >> " << filename << ".asy";
    cmd = ss.str();
    printf("running : %s\n",cmd.c_str());
    std::system(cmd.c_str());
    cmd = "cat plot.asy >> " + filename + ".asy";
    printf("running : %s\n",cmd.c_str());
    std::system(cmd.c_str());
    cmd = "asy " + filename + ".asy";
    printf("running : %s\n",cmd.c_str());
    std::system(cmd.c_str());
    //cmd = "rm " + filename + ".asy";
    //printf("running : %s\n",cmd.c_str());
    //std::system(cmd.c_str());
    //cmd = "convert -density 16 " + filename + ".eps -background white -flatten " + filename + ".png";
    //printf("running : %s\n",cmd.c_str());
    //std::system(cmd.c_str());
    //cmd = "rm " + filename + ".eps";
    //printf("running : %s\n",cmd.c_str());
    //std::system(cmd.c_str());
    //cmd = "rm " + filename;
    //printf("running : %s\n",cmd.c_str());
    //std::system(cmd.c_str());
  }
  
  
  
  
  
  return 0;
}