import graph;
import palette;

real[][] read(string fn)
{
  file in = input(fn);
  real[] raw = in;
  int nx = floor(raw[0]);
  int ny = floor(raw[1]);
  write("reading data of size " + string(ny) + "x" + string(nx));
  real[][] out = new real[ny][nx];
  for(int yi=0;yi!=ny;++yi)
  {
    for(int xi=0;xi!=nx;++xi)
      out[yi][xi] = raw[2 + xi + yi * nx];
  }
  return out;
}


void plotfile(string fn, pair pos)
{
  picture p;
  image(p, read(fn), (0,0), (1,1), Grayscale());
  size(p,5cm, 5cm, point(p,SW), point(p,NE));
  add(p.fit(),pos);
}

plotfile("t1.dat",(0,0));
plotfile("t2.dat",(5.1cm,0));
plotfile("pt1.dat",(0,-5.1cm));
plotfile("pt2.dat",(5.1cm,-5.1cm));
