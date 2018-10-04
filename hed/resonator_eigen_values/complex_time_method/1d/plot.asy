import graph;
file inf = input("dat.dat").word();
real[] raw = inf;

write("raw.size=", raw.length);


picture pe, pb;

real lw=0.03;
int i=0;
while(i < raw.length)
{
  int ne=floor(raw[i]);
  i+=1;
  if(ne == 0)
    break;
  guide ge;
  for(int ii=0;ii!=ne;++ii)
  {
    ge=ge--(ii, lw*raw[i]);
    i+=2;
  }
  draw(pe,ge, red+lw);

  int nb=floor(raw[i]);
  i+=1;
  guide gb;
  for(int ii=0;ii!=nb;++ii)
  {
    gb=gb--(ii, lw*raw[i+1]);
    i+=2;
  }
  draw(pb,gb, red+lw);
  lw+=0.02;
}

picture psp;
real[] lmbd;
while(i < raw.length)
{
  lmbd.push(raw[i]);
  i += 1;
}
guide gsp;
for(int i=0; i!= lmbd.length; ++i)
  gsp = gsp -- (i, lmbd[lmbd.length-1-i]);

draw(psp, gsp);

xaxis(pe, "$x$", BottomTop, LeftTicks);
yaxis(pe, "$E$", LeftRight, RightTicks("$$"));
size(pe, 5cm, 4cm, point(pe, SW), point(pe, NE));
add(pe.fit());

xaxis(pb, "$x$", BottomTop, LeftTicks);
yaxis(pb, "$B$", LeftRight, RightTicks("$$"));
size(pb, 5cm, 4cm, point(pb, SW), point(pb, NE));
add(pb.fit(),(7cm, 0cm));

xaxis(psp, "$i$", BottomTop, LeftTicks);
yaxis(psp, "$\lambda$", LeftRight, RightTicks);
size(psp, 5cm, 4cm, point(psp, SW), point(psp, NE));
add(psp.fit(),(0cm, 7cm));
