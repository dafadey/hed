//file f=input("sg.debug").line();
string[] data=f;
write("size = ",data.length);
unitsize(1mm);
pair[] pts;
real[][] ptsfields;
pair[] edgs;
real[][] edgsfields;
pair[] tris;
real[][] trisfields;
pair[][] polygons;
real[][] polygonfields;
for(int i=0;i!=data.length;++i)
{
  string line=data[i];
  string[] items = split(line," ");
  if(items[0] == "p")
  {
    pts.push(((real) items[1], (real) items[2]));
		real[] fields;
		for(int j=3 ; j < items.length; ++j)
			fields.push((real) items[j]);
		if(fields.length != 0)
			ptsfields.push(fields);
  } 
  if(items[0] == "e")
  {
    edgs.push(((real) items[1], (real) items[2]));
    edgs.push(((real) items[3], (real) items[4]));
		real[] fields;
		for(int j=5 ; j < items.length; ++j)
			fields.push((real) items[j]);
		if(fields.length != 0)
			edgsfields.push(fields);
  }
  if(items[0] == "t")
  {
    tris.push(((real) items[1], (real) items[2]));
    tris.push(((real) items[3], (real) items[4]));
    tris.push(((real) items[5], (real) items[6]));
		real[] fields;
		for(int j=7 ; j < items.length; ++j)
			fields.push((real) items[j]);
		if(fields.length != 0)
			trisfields.push(fields);
  }
	if(items[0] == "box")
	{
		real x0 = (real) items[1];
		real y0 = (real) items[2];
		real x1 = (real) items[3];
		real y1 = (real) items[4];
		pair[] p;
		p.push((x0, y0));
		p.push((x0, y1));
		p.push((x1, y1));
		p.push((x1, y0));
		polygons.push(p);
		real[] fields;
		for(int j=5 ; j < items.length; ++j)
			fields.push((real) items[j]);
		if(fields.length != 0)
			polygonfields.push(fields);
	}
}

write("found " + string(pts.length) + " points");
write(" + " + string(ptsfields.length) + " fields");
write("found " + string(edgs.length/2) + " edges");
write(" + " + string(edgsfields.length) + " fields");
write("found " + string(tris.length/3) + " triangles");
write(" + " + string(trisfields.length) + " fields");
write("found " + string(polygons.length/4) + " polygons");
write(" + " + string(polygonfields.length) + " fields");

pen color(real x)
{
	real s = sqrt(abs(x));
  real r = x > 0 ? s : 0;
  real g = x^2;
  real b = x < 0 ? s : 0;
//	return r * red + g * green + b * blue;
  real gray = 0.5 + 0.5 * (g + r + b) * (r==0 ? -1.0 : 1.0);
  return gray * red + gray * green + gray * blue;
}


pen[] colorbar(int prec = 256)
{
	pen[] res = new pen[prec];
	real k = 1.0 / (prec - 1) * 2.0;
	for(int i = 0; i != prec; ++i)
	{
		real x = i * k - 1.0;
		res[i] = color(x);
	}
	return res;
}

int tfn=0;

real absmax=1.23;

real trimax=-100000;
real trimin=-trimax;

for(int i=0;i!=floor(tris.length/3);++i)
{
  real v = trisfields[i][tfn];
  trimax = max(trimax, v);
  trimin = min(trimin, v);
  pen curcolor = trisfields.length != 0 ? color(v/absmax) : white;
  filldraw((tris[i*3].x,tris[i*3].y)--(tris[i*3+1].x,tris[i*3+1].y)--(tris[i*3+2].x,tris[i*3+2].y)
  --cycle, curcolor, curcolor);
}

write("trimax is ", trimax);
write("trimin is ", trimin);

int efn=0;

/*
for(int i=0;i!=floor(edgs.length/2);++i)
{
	//write(edgsfields[i][efn]);
  draw((edgs[i*2].x,edgs[i*2].y)--(edgs[i*2+1].x,edgs[i*2+1].y), edgsfields.length != 0 ? color(edgsfields[i][efn]) : black);
}


for(int i=0; i != pts.length; ++i)
  dot((pts[i].x,pts[i].y), ptsfields.length != 0 ? color(ptsfields[i][efn]) : black);
*/

real polymax=-100000;
real polymin=-polymax;

for(int i=0; i != polygons.length; ++i)
{
  real v = polygonfields[i][tfn];
  polymax = max(polymax, v);
  polymin = min(polymin, v);
	guide g;
	for(int j=0; j != polygons[i].length; ++j)
		g = g -- polygons[i][j];
	g = g -- cycle;
	//draw(g);
  pen curcolor = polygonfields.length != 0 ? color(v/absmax) : white;
  filldraw(g, curcolor, curcolor+linewidth(1));
}

write("polymax is ", polymax);
write("polymin is ", polymin);
