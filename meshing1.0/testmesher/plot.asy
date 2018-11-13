file f=input("mesh.debug").line();
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

pen color(real v)
{
  return (0.5-0.5*cos(v * pi * 2)) * green
       + (0.5+0.5*cos(v * pi)) * red
       + (0.5-0.5*cos(v * pi)) * blue;
}

element[] elements;
for(int i=0;i!=data.length;++i)
{
  string line=data[i];
  string[] items=split(line," ");
  if(items[0] == "")
    continue;
  if(items[0] == "p")
  {
    pts.push(((real)items[1],(real)items[2]));
    i+=2;
  }
  if(items[0] == "e")
  {
    element edge;
    pair p1=((real)items[1],(real)items[2]);
    pair p2=((real)items[3],(real)items[4]);
    pair l=p2-p1;
    edge.g=((real)items[1],(real)items[2])--((real)items[3],(real)items[4]);
    real ll=sqrt(l.x^2+l.y^2);
    edge.outline = sqrt(ll < 3 ? (3-ll)/3 : 0) * red + sqrt(ll > 3 ? (ll-3 < 3 ? (ll-3)/3 : 1) : 0) * blue;
    edge.fill=nullpen;
//    elements.push(edge);
  }
  if(items[0] == "t")
  {
    element tri;
    tri.g=((real)items[1],(real)items[2])--((real)items[3],(real)items[4])--((real)items[5],(real)items[6])--cycle;
    tri.outline=nullpen;
    if(items.length > 7)
    {
      real v = (real) items[7];
      tri.fill = color(v*7);
      tri.outline = black;
    }
    else
      tri.fill=blue+red*0.7+green*0.8;
    elements.push(tri);
  }
  if(items[0] == "box")
  {
    element box;
    box.g = ((real)items[1],(real)items[2]) -- ((real)items[3],(real)items[2]) -- ((real)items[3],(real)items[4]) -- ((real)items[1],(real)items[4]) -- cycle;
    box.outline=0.5*green;
    box.fill=nullpen;
    elements.push(box);
  }
  if(data[i] == "box_fill")
  {
    element box;
    box.g = ((real)items[1],(real)items[2]) -- ((real)items[3],(real)items[2]) -- ((real)items[3],(real)items[4]) -- ((real)items[1],(real)items[4]) -- cycle;
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
  else if(elements[i].outline==nullpen)
    fill(elements[i].g, elements[i].fill);
  else
    filldraw(elements[i].g, elements[i].fill, elements[i].outline);
}

defaultpen(linewidth(2.5));

for(int i=0;i!=pts.length;++i)
{
	dot(pts[i], 0.5*green);
}
