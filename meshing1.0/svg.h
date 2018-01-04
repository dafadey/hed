#pragma once
#include "primitives.h"
#include <string>
#include <vector>

namespace svg
{
  // what is supported:
  // basically inkscape contours/paths
  // consiting of line segmments and qubic bezier (no shortahand)
  // more specifically m/M c/C l/L z/L keywords are supported
  // 4 svg specs. ref2 http://www.w3.org/TR/SVG/paths.html#PathElement
  //
  // ****input****
  // tolerance:   tolerance T means each time segments will turn by pi/T.
  //              for ex T=180 is fine mesh with smooth 179 corners;
  //              T=12 means a rough surface with 165 degree corners
  //
  // contourID:   contour/path index starting from 0
  //
  // ****output**
  // points of    reference to points returned by function
  // contour:     for svg contours with capital Z specification last point
  //              equal to start point is removed - so no duplicaitions exist         
  //
  // return:      -1 : no such path/contour;
  //               0 : closed contour is read;
  //               1 : a path is read
  int import(std::string /*svg file name*/,
             std::vector<point>& /* ponts of contour/path*/,
             double /* tolerance*/,
             int/*countour ID*/ = 0);
  int importall(std::string /*svg file name*/,
                std::vector<std::vector<point>>& /* contours/paths*/,
                double /* tolerance*/);

};
