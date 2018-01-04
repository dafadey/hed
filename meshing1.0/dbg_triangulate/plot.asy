real do_file(real level, string filename)
{
  file f=input(filename);
  string[] data=f.word();
  pair[] pts;
  pair[] tris;
  pair[] edges;
  for(int i=0;i!=data.length;++i)
  {
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
    if(data[i] == "e")
    {
      edges.push(((real)data[i+1],(real)data[i+2]));
      edges.push(((real)data[i+3],(real)data[i+4]));
      i+=4;
    }
  }
  
  //write(string(tris.length/3)+" triangles");  
  for(int i=0;i!=floor(tris.length/3);++i)
    filldraw(shift(0,level)*((tris[i*3].x,tris[i*3].y)--(tris[i*3+1].x,tris[i*3+1].y)--(tris[i*3+2].x,tris[i*3+2].y)
    --cycle),red+opacity(0.5));

  //write(string(edges.length/2)+" edges");  
  for(int i=0;i!=floor(edges.length/2);++i)
    draw(shift(0,level)*((edges[i*2].x,edges[i*2].y)--(edges[i*2+1].x,edges[i*2+1].y)), blue+linewidth(1.5));
    
  //write(string(pts.length)+" points");  
  for(int i=0;i!=pts.length;++i)
    dot(shift(0,level)*(pts[i].x,pts[i].y),0.5*green);
  //write(point(currentpicture,NE).y/1cm);

  return point(currentpicture,NE).y-point(currentpicture,SE).y;
}

unitsize(0.3cm);

real level=0;
for(int i=0;i!=16;++i)
{
  level = do_file(level, "dump_"+string(i)+".dat");
  //write("level=",level);
}
write("drew results");
