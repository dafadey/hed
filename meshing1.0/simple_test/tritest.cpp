#include "../triangulate.h"
#include <stdio.h>
#include <cstdlib>
#include <vector>
#include <cmath>


/* Crappy code to test triangulation */

bool inside(double x, double y)
{
  if(y<0) return false;
  if(x>0.9 || x<0.1) return false;
  if(y>1.0-double(int(floor(x*3500.0))%500)/1500.0) return false;
  return true;
}

int main(int argc,char **argv)
{
   int i;
   int ntri = 0;
   double x,y;
   std::vector<ITRIANGLE*> v;
   std::vector<XY*> p;
   int nv = 0;
   
   if( argc == 2 )
   {
     FILE *fptr;
     if ((fptr = fopen(argv[1],"r")) == NULL) {
        fprintf(stderr,"Failed to open file \"%s\"\n",argv[1]);
        return -1;
     }
     while (fscanf(fptr,"%lf %lf", &x, &y) == 2) {
        XY* pt = new XY;
        pt->x = x;
        pt->y = y;
        p.push_back(pt);
     }
     fprintf(stderr,"Read %d points\n",nv);
     if (p.size() < 3)
        return -1;
     fclose(fptr);
   }
   else
   {
      nv=10000;
      p.reserve(nv);
      FILE *fptr = fopen("geom.dat","w");
      for(int i=0;i!=nv;i++)
      {
        XY* pt = new XY;
        pt->x=10000;
        pt->y=10000;
        while(!inside(pt->x,pt->y))
        {
          pt->x = (double) rand() / (double) RAND_MAX;
          pt->y = (double) rand() / (double) RAND_MAX;
        }
        p.push_back(pt);
        fprintf(fptr,"p %g %g\n",p[i]->x,p[i]->y);
      }
      fclose(fptr);
   }
   //   qsort(p,nv,sizeof(XY),XYCompare);
   printf("triangulating\n");
   Triangulate(p,v);
   printf("Formed %d triangles\n",v.size());
   printf("Point size is %d\n",p.size());
   
   /* Write triangles in geom format */
   FILE *fptr = fopen("geom.dat","a");
   for (i=0;i<v.size();i++) {
      fprintf(fptr,"t %g %g %g %g %g %g\n",
         v[i]->p1->x,v[i]->p1->y,
         v[i]->p2->x,v[i]->p2->y,
         v[i]->p3->x,v[i]->p3->y);
   }
   fclose(fptr);
   return 0;
}



