import graph;
file f = input("all_lambdas.dat");
real[] raw=f;
//int n=floor(raw[0]);
//int N=floor((raw.length - 1) / n);
//write("n=",n);
//write("N=",N);
picture p;

defaultpen(linewidth(0.3pt));

int ii=0;

pen[] colors={red, 0.6*green, blue, red+blue, 0.5*green+blue, red+0.3*green};

int id=0;
while(ii<raw.length)
{
  int n=floor(raw[ii]);
  ii+=1;
  write(n);
  guide g;
  for(int j = 0; j != n; ++j)
  {
    g = g -- (n - 1 - j, -raw[ii]);
    ii+=1;
  }
  //real id = i / N;
  draw(p, g, colors[id % colors.length]);//, linewidth(0.1pt) + id * red + opacity(id^2));
  id+=1;
}
ylimits(p,0);
xaxis(p, "$i$", BottomTop, LeftTicks("$$",1.0,1.0));
yaxis(p, "$\lambda$", LeftRight, LeftTicks);
size(p, 13cm , 7cm, point(p, SW), point(p, NE));
add(p.fit());
