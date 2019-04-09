// this is isolated subpackage contains of:
// svg.h; svg.cpp; primitives.h; promitives.cpp

#include "svg.h"
#include "primitives.h"
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <iomanip>

//#define VERBOSE

namespace svg
{
  struct bezier_segment
  {
    bezier_segment() : p0(), p1(), p2(), p3() {}
    bezier_segment(point& _p0, point& _p1, point& _p2, point& _p3) : p0(_p0), p1(_p1), p2(_p2), p3(_p3) {}
    point p0,p1,p2,p3;
    point getpoint(double t)
    {
      double t1 = 1.0 - t;
      return p0*t1*t1*t1 + 3.0*p1*t*t1*t1 + 3.0*p2*t*t*t1 + p3*t*t*t;
    }
    point getV(double t)
    {
      double t1 = 1.0 - t;
      return 3.0*(-p0*t1*t1 + p1*t1*(1.0-3.0*t) + p2*t*(2.0-3.0*t) + p3*t*t);
    }
    point geta(double t)
    {
      double t1 = 1 - t;
      return 6.0*(p0*t1 + p1*(3.0*t-2.0) + p2*(1.0-3.0*t) + p3*t);
    }
    double getR(double t)
    {
      point a = geta(t);
      point v = getV(t);
      double ap = (a.x*v.y - a.y*v.x) / v.norm();
      if(ap != 0)
        return std::abs(v.norm2() / ap);
      else
        return std::numeric_limits<double>::max();
    }
  };
  
  void add_bezier_edges(bezier_segment& bz1, double da, double maxdl, std::vector<point>& nodes)
  {
    double t=0;
    while(t<1)
    {
      double dt1, dt2, vmod;
      {
        double r = bz1.getR(t);
        point v = bz1.getV(t);
        vmod = v.norm();
        dt1 = da*r/vmod;
        dt1 = (dt1 + t < 1.0) ? dt1 : 1.0 - t;
      }
      {
        double r = bz1.getR(t + dt1*0.5);
        point v = bz1.getV(t + dt1*0.5);
        vmod = v.norm();
        dt2 = da*r/vmod;
      }
      double dt = std::min(std::abs(dt1) , std::abs(dt2));
      if(dt * vmod > maxdl)
        dt = maxdl/vmod;
      
      //dt=0.05;
      #ifdef VERBOSE
      std::cout << "bz: dt=" << dt << std::endl;
      #endif
      t += dt;
      if(t>1)
        t=1;
      point bzp = bz1.getpoint(t);
      #ifdef VERBOSE
      std::cout << "bz: adding point " << bzp.x << ", " << bzp.y << std::endl;
      #endif
      nodes.push_back(bzp);
    }
    return;
  }
  
  struct bz_input
  {
    enum states {ADD_START_PT, ADD_SEG, ADD_V, ADD_H, ADD_BZ};
    bz_input() : is_relative(false), is_contour(false), state(0), i(0), tol(60) {}
    bool is_relative;
    bool is_contour;
    int state;
    int i;
    double tol;
    point ref_pt;
    point pt;
    bezier_segment bz;
    void feed(const std::string& item, std::vector<point>& nodes)
    {
			#ifdef VERBOSE
			std::cout << "feed line is " << item << std::endl;
      #endif
      if(item=="M")
      {
        state=ADD_START_PT;
        is_relative = false;
			}
      else if (item=="m")
      {
        state=ADD_START_PT;
        is_relative = true;
      }
      else if(item=="v")
      {
        state=ADD_V;
        is_relative = true;
      }
      else if(item=="h")
      {
        state=ADD_H;
        is_relative = true;
      }
      else if(item=="V")
      {
        state=ADD_V;
        is_relative = false;
      }
      else if(item=="H")
      {
        state=ADD_H;
        is_relative = false;
      }
      else if(item=="l")
      {
        state=ADD_SEG;
        is_relative = true;
      }
      else if(item=="L")
      {
        state=ADD_SEG;
        is_relative = false;
      }
      else if(item=="c")
      {
        state=ADD_BZ;
        is_relative = true;
      }
      else if(item=="C")
      {
        state=ADD_BZ;
        is_relative = false;
      }
      else if(item=="z")
      {
        //nodes.erase(nodes.begin() + nodes.size());
        is_contour = true;
      }
      else if(item=="Z")
        is_contour = true;
      else
      {
        std::stringstream ss(item);
        double coord;
        ss >> coord;
        if(state == ADD_START_PT)
        {
          if(i==0)
            pt.x = coord;
          if(i==1)
          {
            pt.y = coord;
            ref_pt = pt;
            #ifdef VERBOSE
            std::cout << "adding master point (" << pt.x << ", " << pt.y << ")" << std::endl;
            #endif
            nodes.push_back(pt);
            state = ADD_SEG;
          }
        }
        else if(state == ADD_SEG)
        {
          if(i==0)
            ref_pt = pt;
          double abs_coord = is_relative ? coord + ((i % 2 == 0) ? ref_pt.x : ref_pt.y) : coord;
          if(i%2==0)
            pt.x = abs_coord;
          if(i%2==1)
          {
            pt.y = abs_coord;
            ref_pt = pt;
            #ifdef VERBOSE
            std::cout << "adding next point (" << pt.x << ", " << pt.y << ")" << std::endl;
            #endif
            nodes.push_back(pt);
          } 
        }
        else if(state == ADD_V)
        {
          double abs_coord = is_relative ? coord + ref_pt.y : coord;
          pt.y = abs_coord;
          ref_pt = pt;
          #ifdef VERBOSE
          std::cout << "adding next point (" << pt.x << ", " << pt.y << ")" << std::endl;
          #endif
          nodes.push_back(pt);
        }
        else if(state == ADD_H)
        {
          double abs_coord = is_relative ? coord + ref_pt.x : coord;
          pt.x = abs_coord;
          ref_pt = pt;
          #ifdef VERBOSE
          std::cout << "adding next point (" << pt.x << ", " << pt.y << ")" << std::endl;
          #endif
          nodes.push_back(pt);
        }
        else if(state == ADD_BZ)
        {
          if(i==0)
          {
            ref_pt = pt;
            bz.p0 = ref_pt;
            #ifdef VERBOSE
            std::cout << "adding bz0 point (" << pt.x << ", " << pt.y << ")" << std::endl;
            #endif
          }
          int ptid = i>>1;
          #ifdef VERBOSE
          std::cout << "adding " << ptid%4 << " (" << i << ") to bezier" << std::endl;
          #endif
          double abs_coord = is_relative ? coord + ((i % 2 == 0) ? ref_pt.x : ref_pt.y) : coord;
          
          if(i%2==0)
            pt.x=abs_coord;
          if(i%2==1) // pt is ready
          {
            pt.y=abs_coord;
            if(ptid % 3 == 0 && i % 2 == 1)
            {
              bz.p0 = ref_pt;
              bz.p1 = pt;
            }
            if(ptid % 3 == 1 && i % 2 == 1)
              bz.p2 = pt;
            if(ptid % 3 == 2 && i % 2 == 1) // bz is ready
            {
              #ifdef VERBOSE
              std::cout << "adding bz3 point (" << pt.x << ", " << pt.y << ")" << std::endl;
              #endif
              bz.p3 = pt;
              ref_pt = pt;
              #ifdef VERBOSE
              std::cout << "generating bezier points" << std::endl;
              #endif
              add_bezier_edges(bz, 3.14/tol, (bz.p3 - bz.p0).norm(), nodes);
            }
          }
        }
        i++;
        return;
      }
      i=0;
      return;
    }
  };
  
  static int parse_path(std::string& path_line, std::vector<point>& cont, double tol)
  {
    bz_input inp;
    inp.tol = tol;
    bool prev_is_separator(false);
    std::string item;
    for(int i = 0; i != path_line.size(); i++)
    {
      char letter = path_line[i];
      bool is_separator = letter == ',' || letter == ' ' || letter == '\t' || letter == '\"';
      if(!is_separator)
        item += letter;
      if(is_separator && !prev_is_separator && i > 0)
      {
        if(item != "d=")
          inp.feed(item, cont);
        item = "";
      }
      prev_is_separator = is_separator;
    }
    return inp.is_contour;
  }
  
  int import(std::string filename, std::vector<point>& nodes, double tol, int poly_id)
  {
    std::ifstream infile(filename);
    std::string line;
    bool inside_path(false);
    bool inside_d(false);
    std::string path_line;
    int path_count(0);
    while(std::getline(infile, line))
    {
      #ifdef VERBOSE
      std::cout << "parsing line" << (inside_path ? "(inside_path) " : " ") << (inside_d ? "(inside_d, path_count"+std::to_string(path_count)+") " : " ") << line << '\n';
      #endif
      if(line.find("<path") != std::string::npos)
      {
        inside_path = true;
        inside_d = false;
      }
      if(inside_path && line.find("=") != std::string::npos)
        inside_d = false;
      if(inside_path && line.find(" d=") != std::string::npos)
        inside_d = true;

      if(inside_d && path_count == poly_id)
        path_line += line;

      if(line.find("/>") != std::string::npos && inside_path)
      {
        path_count++;
        inside_path = false;
        inside_d = false;
      }
    }
    infile.close();
    #ifdef VERBOSE
    std::cout << "input contour line is " << path_line << std::endl;
    #endif
    if(path_line.size() == 0)
      return -1;
    //parsing path_line
    if(parse_path(path_line, nodes, tol))
      return 0;
    else
      return 1;
  }

  bool sfind(std::string& s, std::string what)
  {
    //std::cout << "finding " << what << " in " << s << '\n';
    if(s.size() < what.size())
      return false;
    for(size_t i(0); i != s.size() - what.size() + 1; i++)
    {
      bool found(true);
      for(size_t j(i); j != i + what.size(); j++)
      {
        //std::cout << "\tcomp " << (char) s.c_str()[j] << " vs " << (char) what.c_str()[j - i] << '\n'; 
        if(s.c_str()[j] != what.c_str()[j - i])
        {
          found = false;
          break;
        }
      }
      if(found)
      {
        //std::cout << "FOUND!\n";
        return true;
      }
    }
    return false;
  }

  int importall(std::string filename, std::vector<std::vector<point>>& contours, double tol)
  {
    std::ifstream infile(filename);
    std::string line;
    bool inside_path(false);
    bool inside_d(false);
    std::string path_line;
    int path_count(0);
    while(std::getline(infile, line))
    {
      if(sfind(line,"<path"))
      {
        inside_path = true;
        inside_d = false;
        contours.push_back(std::vector<point>());
        path_line = std::string();
      }
      if(sfind(line,"/>") && inside_path)
      {
        if(parse_path(path_line, contours.back(), tol))
          contours.back().push_back(*(contours.back().begin())); // close if needed
        path_count++;
        inside_path = false;
        inside_d = false;
      }
      if(inside_path && sfind(line,"="))
        inside_d = false;
      if(inside_path && sfind(line," d="))
        inside_d = true;

      if(inside_d)
        path_line += line;
    }
    infile.close();
    for(auto& c : contours)
      std::cout << "found countour of size " << c.size() << std::endl;
    return 0;
  }


  void dump(std::string name, std::vector<std::vector<point>>& vec, std::vector<std::array<unsigned char,4>>* colors)
  {
    std::ofstream s(name.c_str(), std::ios_base::out);
    //write header
    s << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << std::endl;
    s << "<svg" << std::endl;
    s << "    xmlns=\"http://www.w3.org/2000/svg\"" << std::endl; //without this superawesome line mozilla ang google-chrome cannot open the file, but inkscape can!
    s << "    width=\"210mm\"" << std::endl;
    s << "    height=\"297mm\"" << std::endl;
    s << "    viewBox=\"0 0 744.09448819 1052.3622047\">" << std::endl;
    s << "  <g>" << std::endl;
    //write contours
    // filled
    /*
        <path           style="opacity:1;fill:#3aa366;fill-opacity:0.43577983;stroke:#ff0000;stroke-width:0.99999994;stroke-linecap:round;stroke-linejoin:round;stroke-miterlimit:4;stroke-dasharray:1.00000001, 1.99999999;stroke-dashoffset:0;stroke-opacity:1"
       d="m 208.57143,740.93369 231.42857,0 0,87.45826 -231.42857,0 z"/>
    */
    // nonfilled
    /*
        <path       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.99999994px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="M 210.88683,741.32866 438.19888,968.92471 437.71714,741.89043 Z"/>
    */
    int counter(0);
    for(const auto& c : vec)
    {
      #define HEX(x) std::setw(2) << std::setfill('0') << std::hex << (int) x
      if(colors && colors->size() > counter)
        s << "    <path\n       style=\"fill:#" << HEX((*colors)[counter][0]) << HEX((*colors)[counter][1]) << HEX((*colors)[counter][2]) << ";fill-rule:evenodd;fill-opacity:" << (float) (*colors)[counter][3] / 255.0 << ";stroke:#000000;stroke-width:0.99999994px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
      else
        s << "    <path\n       style=\"fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.99999994px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
      s << "       d=\"M";
      for(const auto& p : c)
        s << ' ' << p.x << ',' << p.y;
      s << " Z\"/>\n";
      counter++;
    }
    // finalize
    s << "  </g>" << std::endl;
    s << "</svg>" << std::endl;
    s.close();
  }

};
