#include <iostream>
#include "jacobi_rotations.h"
#include <cmath>

int main()
{
  size_t dim(11);
  algebra::sparse_matrix<double> As;
  for(size_t i(0); i != dim; i++)
    As.add_item(i, i, 1.0);

  size_t col = dim/2;
  size_t row = dim/3;
  As.add_item(col, row, 0.3);
  As.add_item(row, col, 0.3);
  auto Bs = algebra::jacobi_rotate(As, col, row);
  
  algebra::matrix<double> B(dim, dim);
  for(const auto& it : As.data)
    B.set(it[0], it[1], it.val);
  B.dump();
  return 0;
}
