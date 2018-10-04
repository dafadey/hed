import graph;
file f = input("lambdas.dat");
real[] raw=f;
int n=floor(raw[0]);
int N=floor((raw.length - 1) / n);
write("n=",n);
write("N=",N);
picture p;

defaultpen(linewidth(0.3pt));

for(int i = 25; i < N; i += 8)
{
  guide g;
  for(int j = 0; j != n; ++j)
    g = g -- (n - 1 - j, -raw[1 + j + i * n]);
  real id = i / (N -  1);
  draw(p, g, linewidth(0.1pt) + id * red + opacity(id^2));
}
ylimits(p,0);
xaxis(p, "$i$", BottomTop, LeftTicks("$$",1.0,1.0));
yaxis(p, "$\lambda$", LeftRight, LeftTicks);
size(p, 13cm , 7cm, point(p, SW), point(p, NE));
add(p.fit());
