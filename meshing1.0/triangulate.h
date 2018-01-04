#include "elements.h"

int Triangulate(std::vector<XY*>&, std::vector<ITRIANGLE*>&, std::vector<std::array<XY*, 2>>& /*fixed edges*/);

bool check_mesh(std::vector<XY*>& pp, std::vector<ITRIANGLE*>& tt, std::vector<IEDGE*>& ee);
