file f=input("out.dat");
real s0=0.0338797;

string[] data=f;
write("size = ",data.length);

pair[] pts;
pair[] tris;
pair[] edges;

struct element
{
  guide g;
  pen outline;
  pen fill;
};

element[] elements;

for(int i=0;i!=data.length;++i)
{
  string line=data[i];
  //string[] items=split(line," ");
  if(data[i] == "p")
  {
    pts.push(((real)data[i+1],(real)data[i+2]));
    i+=2;
  }
  if(data[i] == "e")
  {
    element edge;
    edge.g=((real)data[i+1],(real)data[i+2])--((real)data[i+3],(real)data[i+4]);
    edge.outline=red;
    edge.fill=nullpen;
    elements.push(edge);
  }
  if(data[i] == "t")
  {
    element tri;
    tri.g=((real)data[i+1],(real)data[i+2])--((real)data[i+3],(real)data[i+4])--((real)data[i+5],(real)data[i+6])--cycle;
    tri.outline=blue;
    tri.fill=nullpen;
    elements.push(tri);
  }
  if(data[i] == "box")
  {
    element box;
    box.g = ((real)data[i+1],(real)data[i+2]) -- ((real)data[i+3],(real)data[i+2]) -- ((real)data[i+3],(real)data[i+4]) -- ((real)data[i+1],(real)data[i+4]) -- cycle;
    box.outline=0.5*green;
    box.fill=nullpen;
    elements.push(box);
  }
  if(data[i] == "box_fill")
  {
    element box;
    box.g = ((real)data[i+1],(real)data[i+2]) -- ((real)data[i+3],(real)data[i+2]) -- ((real)data[i+3],(real)data[i+4]) -- ((real)data[i+1],(real)data[i+4]) -- cycle;
    box.outline=0.5*green;
    box.fill=orange;
    elements.push(box);
  }
}

unitsize(1cm);

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

for(int i=0;i!=pts.length;++i)
{
	dot(pts[i], 0.5*green);
}

for(int i=0;i!=elements.length;++i)
{
  if(elements[i].fill==nullpen)
    draw(elements[i].g, elements[i].outline);
  else
    filldraw(elements[i].g, elements[i].outline, elements[i].fill);
}

