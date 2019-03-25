//file f=input("geom.dat").word();
//real s0=0.0338797;
unitsize(10cm);
string[] data=f;
write("size = ",data.length);
defaultpen(linewidth(0.1));

struct point
{
  pair pt;
  pen outline;
  pen fill; // can make a circle
}


struct element
{
  guide g;
  pen outline;
  pen fill;
  element init()
  {
    element e;
    e.fill=defaultpen;
    e.outline=defaultpen;
    return e;
  }
};

element[] elements;
point[] points;

pen fillcolor = defaultpen;
pen outlinecolor = defaultpen;
for(int i=0;i!=data.length;++i)
{
  string line=data[i];
  //string[] items=split(line," ");
  if(data[i] == "p")
  {
    point pt;
    pt.outline=outlinecolor;
    pt.fill=fillcolor;
    pt.pt=((real)data[i+1],(real)data[i+2]);
    points.push(pt);
    i+=2;
  }
  if(data[i] == "e")
  {
    element edge;
    edge.g=((real)data[i+1],(real)data[i+2])--((real)data[i+3],(real)data[i+4]);
    edge.outline=outlinecolor;
    edge.fill=nullpen;
    elements.push(edge);

    point pt1;
    pt1.outline=outlinecolor;
    pt1.fill=blue;
    pt1.pt=((real)data[i+1],(real)data[i+2]);
    points.push(pt1);

    point pt2;
    pt2.outline=outlinecolor;
    pt2.fill=blue;
    pt2.pt=((real)data[i+3],(real)data[i+4]);
    points.push(pt2);

    i+=4;
  }
  if(data[i] == "t")
  {
    element tri;
    tri.g=((real)data[i+1],(real)data[i+2])--((real)data[i+3],(real)data[i+4])--((real)data[i+5],(real)data[i+6])--cycle;
    tri.outline=outlinecolor;
    tri.fill=nullpen;
    elements.push(tri);
    i+=6;
  }
  if(data[i] == "box")
  {
    element box;
    box.g = ((real)data[i+1],(real)data[i+2]) -- ((real)data[i+3],(real)data[i+2]) -- ((real)data[i+3],(real)data[i+4]) -- ((real)data[i+1],(real)data[i+4]) -- cycle;
    box.outline=outlinecolor;
    box.fill=nullpen;
    elements.push(box);
    i+=4;
  }
  if(data[i] == "box_fill")
  {
    element box;
    box.g = ((real)data[i+1],(real)data[i+2]) -- ((real)data[i+3],(real)data[i+2]) -- ((real)data[i+3],(real)data[i+4]) -- ((real)data[i+1],(real)data[i+4]) -- cycle;
    box.outline=outlinecolor;
    box.fill=fillcolor;
    elements.push(box);
    i+=4;
  }
  if(data[i] == "color_fill")
  {
    fillcolor=((real)data[i+1])*red + ((real)data[i+2])*green + ((real)data[i+3])*blue;
    i+=3;
  }
  if(data[i] == "color_outline")
  {
    outlinecolor=((real)data[i+1])*red + ((real)data[i+2])*green + ((real)data[i+3])*blue;
    i+=3;
  }
  if(data[i] == "outline_width")
  {
    outlinecolor+=linewidth((real)data[i+1]);
    i+=1;
  }
  if(data[i] == "color_no_outline")
    outlinecolor=nullpen;
  if(data[i] == "color_no_fill")
    fillcolor=nullpen;
}

real strain(pair p1, pair p2)
{
  real s=(abs(p2-p1)-s0)/s0;
  if (s>0)
    s=s/(1+s);
  return s;
}

pen color(real s)
{
  return ((s>0)?s^0.5*red:(-s)^0.5*blue) + abs(s)*green;
}

write("elements: ",elements.length);

for(int i=0;i!=elements.length;++i)
{
  if(elements[i].fill==nullpen)
    draw(elements[i].g, elements[i].outline);
  else
    filldraw(elements[i].g, elements[i].fill, elements[i].outline);
}

write("points: ",points.length);

for(int i=0;i!=points.length;++i)
  filldraw(shift(points[i].pt.x,points[i].pt.y)*scale(0.001)*unitcircle, points[i].fill, points[i].outline);
