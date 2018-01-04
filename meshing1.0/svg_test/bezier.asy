
unitsize(10cm);

struct bz
{
  pair p0, p1, p2, p3;
  void operator init(pair _p0, pair _p1, pair _p2, pair _p3)
  {
    this.p0=_p0; this.p1=_p1; this.p2=_p2; this.p3=_p3;
  }
  pair getpoint(real t)
  {
    real t1 = 1-t;
    return p0*t1^3 + 3*p1*t*t1^2 + 3*p2*t^2*t1 + p3*t^3;
  }
  pair getV(real t)
  {
    real t1 = 1-t;
    return 3*(-p0*t1^2 + p1*t1*(1-3*t) + p2*t*(2-3*t) + p3*t^2);
  }
  pair geta(real t)
  {
    real t1 = 1-t;
    return 6*(p0*t1 + p1*(3*t-2) + p2*(1-3*t) + p3*t);
  }
  real getR(real t)
  {
    pair a = geta(t);
    pair v = getV(t);
    real ap = abs(a.x*v.y - a.y*v.x) / sqrt(v.x^2 + v.y^2);
    real r;
    if(abs(ap) != 0)
      r = (v.x * v.x + v.y * v.y) / ap;
    else
      r = -1;
    return r;
  }
};

pair normalized(pair a)
{
  return a/sqrt(a.x^2 + a.y^2);
}

bz bz1=bz((0,0),(1,1),(3.9,0.3),(4,0));
draw(bz1.p0--bz1.p1,red);
draw(bz1.p3--bz1.p2,red);

void drawbz(bz bz1, real da, real maxdl)
{
  real t=0;
  guide bzg;
  while(t<1)
  {
    real r = bz1.getR(t);
    pair v = bz1.getV(t);
    real vmod = sqrt(v.x^2 + v.y^2);
    real dt = da*r/vmod;
    bool reg = false;
    if(dt * vmod > maxdl || dt < 0)
    {
      dt = maxdl/vmod;
      reg = true;
    }
    t += dt;
    if(t>1)
      t=1;
    pair bzp = bz1.getpoint(t);
    bzg=bzg--(bzp);

    dot(bzp, reg ? red : blue);
    draw(bzp--(bzp + 0.1*normalized(bz1.getV(t))),blue);
    draw(bzp--(bzp + 0.1*normalized(bz1.geta(t))),0.5*green);
  }
  draw(bzg);
}

drawbz(bz1,pi/50,0.173);