file f=input("test.dat").word();
string[] data=f;
write("size = ",data.length);

pair[] pts;
pair[] edgs;
pair[] tris;
for(int i=0;i!=data.length;++i)
{
  //string line=data[i];
  //string[] items=split(line," ");
  if(data[i] == "p")
  {
    pts.push(((real)data[i+1],(real)data[i+2]));
    i+=2;
  }
  if(data[i] == "e")
  {
    edgs.push(((real)data[i+1],(real)data[i+2]));
    edgs.push(((real)data[i+3],(real)data[i+4]));
    i+=4;
  }
  if(data[i] == "t")
  {
    tris.push(((real)data[i+1],(real)data[i+2]));
    tris.push(((real)data[i+3],(real)data[i+4]));
    tris.push(((real)data[i+5],(real)data[i+6]));
    i+=6;
  }
}

write("found " + string(pts.length) + " points");
write("found " + string(edgs.length) + " edges");
write("found " + string(tris.length) + " triangles");

unitsize(7cm);
  
for(int i=0;i!=floor(tris.length/3);++i)
{
  filldraw((tris[i*3].x,tris[i*3].y)--(tris[i*3+1].x,tris[i*3+1].y)--(tris[i*3+2].x,tris[i*3+2].y)
  --cycle,red+opacity(0.5),blue);
  //draw((tris[i*3].x,tris[i*3].y)--(tris[i*3+1].x,tris[i*3+1].y)--(tris[i*3+2].x,tris[i*3+2].y)
  //--cycle, blue);
}

for(int i=0;i!=floor(edgs.length/2);++i)
  draw((edgs[i*2].x,edgs[i*2].y)--(edgs[i*2+1].x,edgs[i*2+1].y), .5 * green + .8 * red);

for(int i=0;i!=pts.length;++i)
  dot((pts[i].x,pts[i].y),0.5*green);
