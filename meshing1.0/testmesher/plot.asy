file f=input("mesh.debug").word();
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
    pair p1=((real)data[i+1],(real)data[i+2]);
    pair p2=((real)data[i+3],(real)data[i+4]);
    pair l=p2-p1;
    edge.g=((real)data[i+1],(real)data[i+2])--((real)data[i+3],(real)data[i+4]);
    real ll=sqrt(l.x^2+l.y^2);
    edge.outline = sqrt(ll < 3 ? (3-ll)/3 : 0) * red + sqrt(ll > 3 ? (ll-3 < 3 ? (ll-3)/3 : 1) : 0) * blue;
    edge.fill=nullpen;
    //pts.push(((real)data[i+1],(real)data[i+2]));
    //pts.push(((real)data[i+3],(real)data[i+4]));
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

write("pts size is ", pts.length);
write("elem size is ", elements.length);

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

defaultpen(linewidth(1));

pen color(real s)
{
  return ((s>0)?s^0.5*red:(-s)^0.5*blue) + abs(s)*green;
}

for(int i=0;i!=elements.length;++i)
{
  if(elements[i].fill==nullpen)
    draw(elements[i].g, elements[i].outline);
  else
    filldraw(elements[i].g, elements[i].outline, elements[i].fill);
}

defaultpen(linewidth(2.5));

for(int i=0;i!=pts.length;++i)
{
	dot(pts[i], 0.5*green);
}
