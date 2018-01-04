#include "triangulate.h"
#include <cmath>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <fstream>
#include <iostream>

/*
   Triangulation subroutine
   Takes as input NV vertices in array pxy
   Returned is a list of ntri triangular faces in the array v
   These triangles are arranged in a consistent clockwise order.
   The triangle array 'v' should be malloced to 3 * nv
   The vertex array pxy must be big enough to hold 3 more points
   The vertex array must be sorted in increasing x values say

*/

static int CircumCircle(double, double,
   double, double, double, double, double, double,
   double*, double*, double*);

static int XYCompare(const XY* v1, const XY* v2)
{
  return v1->x < v2->x;
}  

void dump(int id, std::vector<ITRIANGLE*>& v, int ntri, IEDGE* edges, int nedge, std::vector<XY*>& pts, std::vector<std::array<XY*, 2>>& fixed_edges)
{
  std::ofstream out("dump_"+std::to_string(id)+".dat");
  for(int i=0; i!=ntri; i++)
  {
    out << "t\t" << v[i]->p1->x << "\t" << v[i]->p1->y << "\t"
        << v[i]->p2->x << "\t" << v[i]->p2->y << "\t" 
        << v[i]->p3->x << "\t" << v[i]->p3->y << "\t" << std::endl;
  }
  for(int i=0; i!=nedge; i++)
  {
    if (edges[i].p1 == nullptr || edges[i].p2 == nullptr)
      continue;
    out << "e\t" << edges[i].p1->x << "\t" << edges[i].p1->y << "\t" 
        << edges[i].p2->x << "\t" << edges[i].p2->y << std::endl;
  }
  for(auto p : pts)
    out << "p\t" << p->x << "\t" << p->y << std::endl;
  for(auto fe : fixed_edges)
	{
    out << "e\t" << fe[0]->x << "\t" << fe[0]->y << "\t" 
        << fe[1]->x << "\t" << fe[1]->y << std::endl;
	}

  out.close();
}

bool check_mesh(std::vector<XY*>& pp, std::vector<ITRIANGLE*>& tt, std::vector<IEDGE*>& ee)
{
	bool allpok(true);
	for(auto p : pp)
	{
		bool pok(true);
		for(auto t : p->tris)
		{
			bool tok(false);
			for(int i(0); i != 3; i++)
			{
				if(t->point(i) == p)
				{
					tok = true;
					break;
				}
			}
			if(!tok)
				pok = false;
		}
		if(!pok)
			allpok = false;
	}
	if(!allpok)
		std::cout << "points are linked to wrong triangles" << std::endl;

	bool alltok(true);
	for(auto t : tt)
	{
		bool tok(true);
		for(int i(0); i != 3; i++)
		{
			bool ok(false);
			for(auto pt : t->point(i)->tris)
			{
				if(pt == t)
				{
					ok = true;
					break;
				}
			}
			if(!ok)
				tok = false;
		}
		if(!tok)
			alltok = false;
	}
	if(!alltok)
		std::cout << "some triangles are not linked to their points" << std::endl;

	return allpok && alltok;
} 

void check_flip_edge(std::array<XY*,2>& fe);// this is tocorrectedges that cross fixed_edges. eats fixed_edge.

int Triangulate(std::vector<XY*>& pxy, std::vector<ITRIANGLE*>& v, std::vector<std::array<XY*, 2>>& fixed_edges)
{
  int nv = pxy.size();
  v.reserve(3*nv);
  std::sort(pxy.begin(), pxy.end(),XYCompare);
  int *complete = NULL;
  IEDGE *edges = NULL;

  int nedge = 0;
  int ntri = 0;
  int trimax,emax = 200;

  int inside;
  int i,j,k;
  double xp,yp,x1,y1,x2,y2,x3,y3,xc,yc,r;
  double xmin,xmax,ymin,ymax,xmid,ymid;
  double dx,dy,dmax;

  /* Allocate memory for the completeness list, flag for each triangle */
  trimax = 4 * nv;
  if ((complete = (int*) std::malloc(trimax*sizeof(int))) == NULL) {
    free(edges);
    free(complete);
    return 1;
  }

  /* Allocate memory for the edge list */
  if ((edges = (IEDGE*) std::malloc(emax*(long)sizeof(IEDGE))) == NULL) {
    free(edges);
    free(complete);
    return 2;
  }

  /*
    Find the maximum and minimum vertex bounds.
    This is to allow calculation of the bounding triangle
  */
  xmin = pxy[0]->x;
  ymin = pxy[0]->y;
  xmax = xmin;
  ymax = ymin;
  for (i=1;i<nv;i++)
  {
    if (pxy[i]->x < xmin) xmin = pxy[i]->x;
    if (pxy[i]->x > xmax) xmax = pxy[i]->x;
    if (pxy[i]->y < ymin) ymin = pxy[i]->y;
    if (pxy[i]->y > ymax) ymax = pxy[i]->y;
  }
  dx = xmax - xmin;
  dy = ymax - ymin;
  dmax = (dx > dy) ? dx : dy;
  xmid = (xmax + xmin) / 2.0;
  ymid = (ymax + ymin) / 2.0;

  /*
    Set up the supertriangle
    This is a triangle which encompasses all the sample points.
    The supertriangle coordinates are added to the end of the
    vertex list. The supertriangle is the first triangle in
    the triangle list.
  */
  XY super[3];
  super[0].x = xmid - 2 * dmax;
  super[0].y = ymid - dmax;
  super[1].x = xmid;
  super[1].y = ymid + 2 * dmax;
  super[2].x = xmid + 2 * dmax;
  super[2].y = ymid - dmax;
  pxy.push_back(&super[0]);
  pxy.push_back(&super[1]);
  pxy.push_back(&super[2]);
  ITRIANGLE* supert = new ITRIANGLE;
  supert->p1 = pxy[nv];
  supert->p2 = pxy[nv+1];
  supert->p3 = pxy[nv+2];
  v[0] = supert;
  complete[0] = false;
  ntri=1;

  /*
    Include each point one at a time into the existing mesh
  */
  for (i=0;i<nv;i++)
  {
    xp = pxy[i]->x;
    yp = pxy[i]->y;
    nedge = 0;

    /*
       Set up the edge buffer.
       If the point (xp,yp) lies inside the circumcircle then the
       three edges of that triangle are added to the edge buffer
       and that triangle is removed.
    */
    for (j = 0; j < ntri; j++)
    {
      if (complete[j])
        continue;
      x1 = v[j]->p1->x;
      y1 = v[j]->p1->y;
      x2 = v[j]->p2->x;
      y2 = v[j]->p2->y;
      x3 = v[j]->p3->x;
      y3 = v[j]->p3->y;
      inside = CircumCircle(xp, yp, x1, y1, x2, y2, x3, y3, &xc, &yc, &r);
      if (xc < xp && ((xp - xc) * (xp - xc)) > r)
        complete[j] = true;
      if (inside)
      {
        /* Check that we haven't exceeded the edge list size */
        if (nedge+3 >= emax)
        {
          emax += 100;
          if ((edges = (IEDGE*) std::realloc(edges,emax*(long)sizeof(IEDGE))) == NULL) 
          {
            free(edges);
            free(complete);
            return 3;
          }
        }
        edges[nedge+0].p1 = v[j]->p1;
        edges[nedge+0].p2 = v[j]->p2;
        edges[nedge+1].p1 = v[j]->p2;
        edges[nedge+1].p2 = v[j]->p3;
        edges[nedge+2].p1 = v[j]->p3;
        edges[nedge+2].p2 = v[j]->p1;
        nedge += 3;
        if(j != ntri-1)
        {
          delete v[j];
          v[j] = v[ntri-1];
          v[ntri-1] = nullptr;
        }
        complete[j] = complete[ntri-1];
        ntri--;
        j--;
      }
    }

    /*
       Tag multiple edges (remove shared edges)
       Note: if all triangles are specified anticlockwise then all
             interior edges are opposite pointing in direction.
    */
    for (j=0;j<nedge-1;j++)
    {
      for (k=j+1;k<nedge;k++)
      {
        if ((edges[j].p1 == edges[k].p2) && (edges[j].p2 == edges[k].p1))
        {
          edges[j].p1 = nullptr;
          edges[j].p2 = nullptr;
          edges[k].p1 = nullptr;
          edges[k].p2 = nullptr;
        }
        /* Shouldn't need the following, see note above */
        if ((edges[j].p1 == edges[k].p1) && (edges[j].p2 == edges[k].p2))
        {
          edges[j].p1 = nullptr;
          edges[j].p2 = nullptr;
          edges[k].p1 = nullptr;
          edges[k].p2 = nullptr;
        }
      }
    }

    /*
       Form new triangles for the current point
       Skipping over any tagged edges.
       All edges are arranged in clockwise order.
    */
    for (j=0;j<nedge;j++)
    {
      if (edges[j].p1 == nullptr || edges[j].p2 == nullptr)
        continue;
      if (ntri >= trimax)
      {
        free(edges);
        free(complete);
        return 4;
      }
      v[ntri] = new ITRIANGLE;
      v[ntri]->p1 = edges[j].p1;
      v[ntri]->p2 = edges[j].p2;
      v[ntri]->p3 = pxy[i];
      complete[ntri] = false;
      ntri++;
    }

//    dump(i,v,ntri,edges,nedge, pxy, fixed_edges);

  }

  /*
    Remove triangles with supertriangle vertices
    These are triangles which have a vertex number greater than nv
  */
  for (i=0;i<ntri;i++)
  {
    if (v[i]->p1 == &super[0] || v[i]->p1 == &super[1] || v[i]->p1 == &super[2] ||
        v[i]->p2 == &super[0] || v[i]->p2 == &super[1] || v[i]->p2 == &super[2] ||
        v[i]->p3 == &super[0] || v[i]->p3 == &super[1] || v[i]->p3 == &super[2] )
    {
      if(i!=ntri-1)
      {
        delete v[i];
        v[i] = v[ntri-1];
        v[ntri-1] = nullptr;
      }
      ntri--;
      i--;
    }
  }
  v.erase(v.begin()+ntri, v.end());
  pxy.erase(pxy.begin()+nv, pxy.end());
  // flip triangles to met fixed_edges condition
  // (NOTE: Delaunay condition may be broken)
  for(int i = 0; i != ntri; i++)
  {
		for(int j = 0; j != 3; j++)
			v[i]->point(j)->tris.push_back(v[i]);
	}
	
	std::vector<IEDGE*> edump;
	
	if(check_mesh(pxy, v, edump))
		std::cout << "mesh is ok" << std::endl;
	else
		std::cout << "mesh is bad" << std::endl;
	
	// we can make edges but... let us try solve the problem with only point->triangle links
	// guess it will be a bit slower
	for(auto& fe : fixed_edges)
		check_flip_edge(fe);

	if(check_mesh(pxy, v, edump))
		std::cout << "mesh is ok" << std::endl;
	else
		std::cout << "mesh is bad" << std::endl;


  //dump(pxy.size(),v,ntri,edges,0, pxy, fixed_edges);

	return 0;
}

static bool segs_cross(const XY& s0p0, const XY& s0p1, const XY& s1p0, const XY& s1p1)
{
	if(vp(s1p0 - s0p0, s0p1 - s0p0) * vp(s1p1 - s0p0, s0p1 - s0p0) > 0.0)
		return false;
	if(vp(s0p0 - s1p0, s1p1 - s1p0) * vp(s0p1 - s1p0, s1p1 - s1p0) > 0.0)
		return false;
	return true;
}

void check_flip_edge(std::array<XY*,2>& fe)
{
	ITRIANGLE* crossing_edge_triangle(nullptr);
	bool good(false);
	XY* outer_edge[2];
	for(auto t : fe[0]->tris)
	{
		int vert(0);
		for(int i = 0; i != 3; i++)
		{
			auto tp(t->point(i));
			if(tp == fe[1])
			{
				good = true;
				break;
			}
			if(tp != fe[0])
			{
				outer_edge[vert] = tp; 
				vert++;
			}
		}
		if(good)
			break;
		if(vert != 2)
		{
			std::cerr << "MESHING ERROR: could not found two vertices not coinciding with fixed_edge first vertex (vert=" << vert << ")" << std::endl;
			break;
		}
		//check if edge crosses fixed_edge
		if(!segs_cross(*fe[0], *fe[1], *outer_edge[0], *outer_edge[1]))
			continue;
		crossing_edge_triangle = t; // ahha! we have finally found the edge that we want to flip
		break;
	}
	if(good) //no need to flip anythig
		return;
	if(!crossing_edge_triangle)
	{
		std::cerr << "MESHING ERROR: fixed edge is not good but the mesh edge that need to be flipped to match fixed_edge was not found" << std::endl;
		return;
	}
	//flipping found corresponding triangle neighboring fe[1]:
	ITRIANGLE* crossing_co_edge_triangle(nullptr);
	for(auto t : fe[1]->tris)
	{
		XY* co_outer_edge[2];
		int vert(0);
		for(int i = 0; i != 3; i++)
		{
			auto tp(t->point(i));
			if(tp != fe[1])
			{
				co_outer_edge[vert] = tp; 
				vert++;
			}
		}
		if(co_outer_edge[0] == outer_edge[0] && co_outer_edge[1] == outer_edge[1])
		{
			crossing_co_edge_triangle = t;
			break;
		}
		if(co_outer_edge[0] == outer_edge[1] && co_outer_edge[1] == outer_edge[0])
		{
			crossing_co_edge_triangle = t;
			break;
		}
	}
	if(!crossing_co_edge_triangle)
	{
		std::cerr << "MESHING ERROR: did not found triangle crossing same fixed_edge attached to second vertex of fixed_edge" << std::endl;
		return;
	}
	//ok we need to remove this two (crossing_edge_triangle and crossing_co_edge_triangle) triangles
	//and create flipped triangles

	//chek direction
	double s = crossing_edge_triangle->area();
	crossing_edge_triangle->p1 = fe[0];
	crossing_edge_triangle->p2 = fe[1];
	crossing_edge_triangle->p3 = outer_edge[0];
	if(crossing_edge_triangle->area() * s < 0.0)
	{
		crossing_edge_triangle->p1 = fe[1];
		crossing_edge_triangle->p2 = fe[0];
		crossing_edge_triangle->p3 = outer_edge[0];
	}
	s = crossing_co_edge_triangle->area();
	crossing_co_edge_triangle->p1 = fe[0];
	crossing_co_edge_triangle->p2 = fe[1];
	crossing_co_edge_triangle->p3 = outer_edge[1];
	if(crossing_co_edge_triangle->area() * s < 0.0)
	{
		crossing_co_edge_triangle->p1 = fe[1];
		crossing_co_edge_triangle->p2 = fe[0];
		crossing_co_edge_triangle->p3 = outer_edge[1];
	}
	//AND RELINK POINTS!!!
	//clean up crossing_edge_triangle and crossing_co_edge_triangle triangles from XY::tris
	XY* pts[4] = {fe[0], fe[1], outer_edge[0], outer_edge[1]};
	for(auto p : pts)
	{
		std::vector<ITRIANGLE*> new_tris;
		for(auto t : p->tris)
		{
			if(t == crossing_co_edge_triangle)
				continue;
			if(t == crossing_edge_triangle)
				continue;
			new_tris.push_back(t);
		}
		p->tris = new_tris;
	}
	//add crossing_edge_triangle and crossing_co_edge_triangle to correct places in XY::tris
	for(int i = 0; i != 3; i++)
	{
		crossing_edge_triangle->point(i)->tris.push_back(crossing_edge_triangle);
		crossing_co_edge_triangle->point(i)->tris.push_back(crossing_co_edge_triangle);
	}
}

/*
   Return true if a point (xp,yp) is inside the circumcircle made up
   of the points (x1,y1), (x2,y2), (x3,y3)
   The circumcircle centre is returned in (xc,yc) and the radius r
   NOTE: A point on the edge is inside the circumcircle
*/
#define EPSILON 1e-7
static int CircumCircle(double xp,double yp,
   double x1,double y1,double x2,double y2,double x3,double y3,
   double *xc,double *yc,double *rsqr)
{
   double m1,m2,mx1,mx2,my1,my2;
   double dx,dy,drsqr;
   double fabsy1y2 = fabs(y1-y2);
   double fabsy2y3 = fabs(y2-y3);

   /* Check for coincident points */
   if (fabsy1y2 < EPSILON && fabsy2y3 < EPSILON)
       return(false);

   if (fabsy1y2 < EPSILON) {
      m2 = - (x3-x2) / (y3-y2);
      mx2 = (x2 + x3) / 2.0;
      my2 = (y2 + y3) / 2.0;
      *xc = (x2 + x1) / 2.0;
      *yc = m2 * (*xc - mx2) + my2;
   } else if (fabsy2y3 < EPSILON) {
      m1 = - (x2-x1) / (y2-y1);
      mx1 = (x1 + x2) / 2.0;
      my1 = (y1 + y2) / 2.0;
      *xc = (x3 + x2) / 2.0;
      *yc = m1 * (*xc - mx1) + my1;
   } else {
      m1 = - (x2-x1) / (y2-y1);
      m2 = - (x3-x2) / (y3-y2);
      mx1 = (x1 + x2) / 2.0;
      mx2 = (x2 + x3) / 2.0;
      my1 = (y1 + y2) / 2.0;
      my2 = (y2 + y3) / 2.0;
      *xc = (m1 * mx1 - m2 * mx2 + my2 - my1) / (m1 - m2);
      if (fabsy1y2 > fabsy2y3) {
         *yc = m1 * (*xc - mx1) + my1;
      } else {
         *yc = m2 * (*xc - mx2) + my2;
      }
   }

   dx = x2 - *xc;
   dy = y2 - *yc;
   *rsqr = dx*dx + dy*dy;

   dx = xp - *xc;
   dy = yp - *yc;
   drsqr = dx*dx + dy*dy;

   // Original
   //return((drsqr <= *rsqr) ? true : false);
   // Proposed by Chuck Morris
   return((drsqr - *rsqr) <= EPSILON ? true : false);
}
#undef EPSILON


