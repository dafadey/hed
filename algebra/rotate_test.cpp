#include <iostream>
#include "jacobi_rotations.h"
#include <cmath>

int main()
{
  size_t dim(11);
  algebra::sparse_matrix<double> As;
  for(size_t i(0); i != dim; i++)
    As.add_item(i, i, 1.1);


  size_t col = dim/2+1;
  size_t row = dim/2;
  As.add_item(col, row, 0.3);
  As.add_item(row, col, 0.3);
  As.add_item(col+1, row+1, 0.4);
  As.add_item(row+1, col+1, 0.4);

  As.make_rows_and_cols();

  algebra::matrix<double> A(dim, dim);
  for(const auto& it : As.data)
    A.set(it[0], it[1], it.val);
  std::cout << "initial matrix:\n";
  A.dump();


  auto Bs = algebra::jacobi_rotate(As, col, row);
  
  algebra::matrix<double> B(dim, dim);
  for(const auto& it : Bs.data)
    B.set(it[0], it[1], it.val);
  std::cout << "resulting matrix:\n";
  B.dump("B");
  Bs.dump("Bs");
  //test
  for(const auto& itm : Bs.data)
  {
    if(itm[0] != itm[1] && std::abs(itm.val) > 1.e-8)
    {
      std::cout << "test failed\n";
      return -1;
    }
  }
  std::cout << "test passed\n";
  return 0;
}
