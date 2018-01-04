file f=input("geom.dat");
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

unitsize(30cm);

  
for(int i=0;i!=floor(tris.length/3);++i)
  filldraw((tris[i*3].x,tris[i*3].y)--(tris[i*3+1].x,tris[i*3+1].y)--(tris[i*3+2].x,tris[i*3+2].y)
  --cycle,red+opacity(0.5),blue);
  
for(int i=0;i!=pts.length;++i)
  dot((pts[i].x,pts[i].y),0.5*green);
