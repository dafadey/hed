#include <svg.h>
#include <fstream>

int main()
{
  std::vector<std::vector<point>> contours;
  svg::importall("input.svg", contours, 0.1);
  std::ofstream of("input.poly");
  int ci(0);
  for(const auto& c : contours)
  {
    of << ci << "\t" << c.size();
    for(const auto& p : c)
      of << "\t" << p.x << "\t" << p.y;
    of << "\n";
    ci++;
  }
  of.close();
  return 0;
}
