#include <iostream>
#include "jacobi_rotations.h"
#include <cmath>

int main()
{
  size_t dim(13);
  algebra::sparse_matrix<double> As;
  for(size_t i(0); i != dim; i++)
    As.add_item(i, i, -2.0);//1.1 - 0.1 * std::abs((double) i - 0.5 * (double) dim) / (double) dim);
  for(size_t i(0); i != dim - 1; i++)
  {
    As.add_item(i, i + 1, 1.0);
    As.add_item(i + 1, i, 1.0);
  }

  algebra::matrix<double> A(dim, dim);
  for(const auto& it : As.data)
    A.set(it[0], it[1], it.val);
  std::cout << "initial matrix:\n";
  A.dump();

  auto Bs = algebra::jacobi_diag(As);
  
  algebra::matrix<double> B(dim, dim);
  for(const auto& it : Bs.data)
    B.set(it[0], it[1], it.val);
  std::cout << "resulting matrix:\n";
  B.dump();
  
  std::vector<double> spectra;
  for(const auto& itm : Bs.data)
  {
    if(itm[0] == itm[1])
      spectra.emplace_back(itm.val);
  }
  sort(spectra.begin(), spectra.end());
  for(const auto& s : spectra)
    std::cout << s << ", ";
  std::cout << '\n';
  
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
