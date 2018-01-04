//file f=input("geom.dat");
//real s0=0.0338797;

string[] data=f;
write("size = ",data.length);

pair[] pts;
pair[] tris;
for(int i=0;i!=data.length;++i)
{
  string line=data[i];
  //string[] items=split(line," ");
  if(data[i] == "p")
  {
    pts.push(((real)data[i+1],(real)data[i+2]));
    i+=2;
  }
  if(data[i] == "t")
  {
    tris.push(((real)data[i+1],(real)data[i+2]));
    tris.push(((real)data[i+3],(real)data[i+4]));
    tris.push(((real)data[i+5],(real)data[i+6]));
    i+=6;
  }
}

unitsize(100cm);

//for(int i=0;i!=pts.length;++i)
//  dot((pts[i].x,pts[i].y),red);

real strain(pair p1, pair p2)
{
  real s=(abs(p2-p1)-s0)/s0;
  if (s>0)
    s=s/(1+s);
  return s;
}

defaultpen(linewidth(5));

pen color(real s)
{
  return ((s>0)?s^0.5*red:(-s)^0.5*blue) + abs(s)*green;
}

void drawedge(pair p1, pair p2)
{
  draw(p1--p2,color(strain(p1,p2)));
}
 
for(int i=0;i!=floor(tris.length/3);++i)
{
  drawedge((tris[i*3].x,tris[i*3].y),(tris[i*3+1].x,tris[i*3+1].y));
  drawedge((tris[i*3+1].x,tris[i*3+1].y),(tris[i*3+2].x,tris[i*3+2].y));
  drawedge((tris[i*3+2].x,tris[i*3+2].y),(tris[i*3].x,tris[i*3].y));
}
//for(int i=0;i!=floor(tris.length/3);++i)
//  filldraw((tris[i*3].x,tris[i*3].y)--(tris[i*3+1].x,tris[i*3+1].y)--(tris[i*3+2].x,tris[i*3+2].y)--cycle,red,blue);