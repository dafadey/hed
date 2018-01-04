file f=input(fn).word();
write("hi this is asy, reading file " + fn);
real[] raw=f;
write(raw);
pair[][] contours;
real x, y;
int cid=-1;
int stage=0;
int id;
int n;
pen[] colors;
for(int i=0; i!=raw.length; ++i)
{
  if(stage == 0)
  {
    id = floor(raw[i]);
    stage+=1;
    contours.push(new pair[]);
    cid+=1;
  }
  else if(stage == 1)
  {
    n = floor(raw[i]);
    if(n!=0)
      stage+=1;
    else
      stage=0;
    write("n=",n);
    write("stage=",stage);
  }
  else if(stage >= 2)
  {
    write("   stage=",stage);
    if(stage % 2 == 0)
      x = raw[i];
    else
    {
      y = raw[i];
      write("point: ",(x,y));
      contours[cid].push((x, y));
      if(stage - 1 == n * 2)
        stage=-1;
    }
    stage+=1;
  }
}

write("contours length is ", contours.length);
write("cid is ", cid);
for(int i=0; i!=cid+1; ++i)
{
  guide g;
  write("writing contour of size ", contours[i].length);
  for(int j=0; j!=contours[i].length; ++j)
  {
    write("\t",contours[i][j]);
    g=g--(contours[i][j]);
  }
  //g=g--cycle;
  draw(g);
}
