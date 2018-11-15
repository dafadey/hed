#include "resonators.h"

double resonator_base::norm()
{
  double n2(.0);
  for(const auto& e : E)
    n2 = std::max(n2, std::norm(e));
  for(const auto& b : B)
    n2 = std::max(n2, std::norm(b));
  n2 = 1.0 / sqrt(n2);
  for(auto& e : E)
    e *= n2;
  for(auto& b : B)
    b *= n2;
  return log(n2);
}
  
void resonator_base::copy_fields(const resonator_base* from)
{
  E.clear();
  B.clear();

  for(auto& e : from->E)
    E.emplace_back(e);
    
  for(auto& b : from->B)
    B.emplace_back(b);
}
