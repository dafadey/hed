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
	}
}

write("found " + string(pts.length) + " points");
write(" + " + string(ptsfields.length) + " fields");
write("found " + string(edgs.length/2) + " edges");
write(" + " + string(edgsfields.length) + " fields");
write("found " + string(tris.length/3) + " triangles");
write(" + " + string(trisfields.length) + " fields");

pen color(real x)
{
	real s = sqrt(abs(x));
	return (x > 0 ? red * s : red * 0) + green * x^2 + (x < 0 ? blue * s : blue * 0);
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

for(int i=0;i!=floor(tris.length/3);++i)
{
  fill((tris[i*3].x,tris[i*3].y)--(tris[i*3+1].x,tris[i*3+1].y)--(tris[i*3+2].x,tris[i*3+2].y)
  --cycle, trisfields.length != 0 ? color(trisfields[i][tfn]) : white);
}

int efn=0;

for(int i=0;i!=floor(edgs.length/2);++i)
{
	//write(edgsfields[i][efn]);
  draw((edgs[i*2].x,edgs[i*2].y)--(edgs[i*2+1].x,edgs[i*2+1].y), edgsfields.length != 0 ? color(edgsfields[i][efn]) : black);
}

for(int i=0; i != pts.length; ++i)
  dot((pts[i].x,pts[i].y), ptsfields.length != 0 ? color(ptsfields[i][efn]) : black);

for(int i=0; i != polygons.length; ++i)
{
	guide g;
	for(int j=0; j != polygons[i].length; ++j)
		g = g -- polygons[i][j];
	g = g -- cycle;
	draw(g);
}
