import graph;
file in=input("stats.dat").line();

defaultpen(fontsize(8pt));

string[] sraw = in;

struct metrics
{
  int n;
  int bndry;
  real p_sz;
  real R;
}

metrics[] KD;
metrics[] G;

for(int i=0; i!=sraw.length; ++i)
{
  string[] items = split(sraw[i]);
  metrics q;
  q.n = (int) items[1];
  q.bndry = (int) items[2];
  q.p_sz = (real) items[3];
  q.R = (real) items[4];
  if(items[0]=="KD")
    KD.push(q);
  else if(items[0]=="G")
    G.push(q);
  else
    write("unknown key ", items[0]);
}
write("read " + string(KD.length) + " KD metrics");
write("read " + string(G.length) + " G metrics");

picture p;

real bar=0.7;
guide[] ebar = {((-0.5*bar,-0.5)--(0.5*bar,-0.5)--(0,-0.5)--(0,0.5)--(0.5*bar,0.5)--(-0.5*bar,0.5)),
                ((-0.5*bar,-0.5)--(0.5*bar,-0.5)--(0.5*bar,0.5)--(-0.5*bar,0.5)--cycle)};


marker my_marker(int type, pen pn)
{
  return marker(scale(0.8mm)*rotate(type*45)*shift(-0.5,-0.5)*unitsquare, pn, FillDraw(pn),above=true);
}
                
void draw_stats(metrics[] mt, pen pn, int type)
{
  for(int i=0; i!= mt.length; ++i)
    draw(p, shift(mt[i].n, mt[i].p_sz) * scale(1.0, mt[i].R) * ebar[type], pn);
  
  guide g;
  for(int i=0; i!= mt.length; ++i)
    g=g--(mt[i].n, mt[i].bndry);
  draw(p, g, 0.5*gray+0.5*white, my_marker(type, pn));

}

draw_stats(KD, red, 0);

draw_stats(G, blue, 1);

pair center=(27,670);

draw(p,shift(center.x, center.y)*scale(1,100)*ebar[0],red);
draw(p,shift(center.x-2,center.y)*scale(1,100)*ebar[1], blue);
label(p,"--", (center.x+1pt,center.y), E);
label(p,"$<\!n_t\!>^{parts}$", (center.x+1pt+2pt,center.y+42), E);
label(p,"$\sigma$", (center.x+1pt+2pt,center.y-42), E);

center+=(0,-130);

draw(p,(center.x,center.y), my_marker(0, red));
draw(p,(center.x-2,center.y), my_marker(1, blue));
label(p,"-- $\Sigma$ length", (center.x+1pt,center.y), E);

center+=(0,-130);

draw(p,shift(center.x-2,center.y)*scale(1,100)*ebar[1],blue);
draw(p,(center.x,center.y), my_marker(1, blue));
label(p,"-- G(greedy)", (center.x+1pt,center.y), E);

center+=(7,-130);

draw(p,shift(center.x-2,center.y)*scale(1,100)*ebar[0],red);
draw(p,(center.x,center.y), my_marker(0, red));
label(p,"-- KD", (center.x+1pt,center.y), E);


xlimits(p,0, 45);
ylimits(p,0,1150);
xaxis(p, "$n_{parts}$", BottomTop, LeftTicks);
yaxis(p, "", LeftRight, RightTicks);


size(p, 4cm, 4cm, point(p, SW), point(p, NE));
add(p.fit());

