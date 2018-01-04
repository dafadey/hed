#include "../triangulate.h"
#include <stdio.h>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <iostream>

int main(int argc,char **argv)
{
   int i;
   int ntri = 0;
   double x,y;
   std::vector<ITRIANGLE*> v;
   std::vector<XY*> p;
   int nv = 0;
   for(int i=0;i<10;i++)
   {
     XY* pt = new XY;
     pt->x = (double) rand() / (double) RAND_MAX * 10.0;
     pt->y = (double) rand() / (double) RAND_MAX * 10.0;
     p.push_back(pt);
   }
   std::vector<std::array<XY*,2>> fixed_edges;
   for(int i=0;i<5;i++)
   {
     XY* pt = new XY;
     pt->x = 10 - double(i+1) / 5.0 * 6.6;
     pt->y = double(i+1) / 5.0 * 10.0;
     XY* e1_ptr = p.back();
     p.push_back(pt);
     XY* e2_ptr = p.back();
     if(i > 0)
			fixed_edges.push_back(std::array<XY*,2>{{e1_ptr, e2_ptr}});
   }
   
   Triangulate(p,v, fixed_edges);
   std::cout << "Ok" << std::endl;
}
