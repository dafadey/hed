#include <heds/solver.h>
#include <svg.h>

int main()
{
  std::vector<std::vector<point>> contours;
  svg::importall("input_geo.svg", contours, 0.1);
  return 0;
}
