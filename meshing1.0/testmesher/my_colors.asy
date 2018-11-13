pen color(real v)
{
  return (0.5-0.5*cos(v * pi * 2)) * green
       + (0.5+0.5*cos(v * pi)) * red
       + (0.5-0.5*cos(v * pi)) * blue;}
for(int i=0; i<100; ++i)
  fill(shift(i)*unitsquare, color(i/99));
