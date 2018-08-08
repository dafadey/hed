#include <iostream>
#include "mul.h"
#include "sparse.h"
#include "matrix.h"

int main()
{
  /*
  1.1  1.3  1.5
  1.2  1.4  1.6
  1.22 1.43 1.73
  */
  
  std::cout << "regular matrix set/read/print test:\n";
  algebra::matrix<double> A(3, 2);
  A.set(0,0,1.1);
  A.set(0,1,1.2);
  //A.set(0,2,1.22);
  A.set(1,0,1.3);
  A.set(1,1,1.4);
  //A.set(1,2,1.43);
  A.set(2,0,1.5);
  A.set(2,1,1.6);
  //A.set(2,2,1.73);
  A.dump("A", "\t\t");

  algebra::matrix<double> B(2, 3);
  B.set(0,0,1.1);
  B.set(0,1,1.2);
  B.set(0,2,1.22);
  B.set(1,0,1.3);
  B.set(1,1,1.4);
  B.set(1,2,1.43);
  //B.set(2,0,1.5);
  //B.set(2,1,1.6);
  //B.set(2,2,1.73);
  B.dump("B", "\t\t");

  auto C = A * B;
  C.dump("A * B", "\t\t");
  
  std::cout << "sparse matrix set/read/print test:\n";
  algebra::sparse_matrix<double> As;
  for(size_t c(0); c != A.dimx; c++)
  {
    for(size_t r(0); r != A.dimy; r++)
      As.add_item(A.dimx - c - 1, A.dimy - r - 1, A.get(A.dimx - c - 1, A.dimy - r - 1));
  }
  As.dump("As", "\t\t");
  As.make_rows_and_cols();
  As.dump("As", "\t\t");

  algebra::sparse_matrix<double> Bs;
  for(size_t c(0); c != B.dimx; c++)
  {
    for(size_t r(0); r != B.dimy; r++)
      Bs.add_item(c, r, B.get(c, r));
  }
  Bs.dump("Bs", "\t\t");
  Bs.make_rows_and_cols();
  Bs.dump("Bs", "\t\t");

  auto Cs = As * Bs;
  Cs.dump("As * Bs", "\t\t");
  
  //the test
  bool failed(false);
  for(size_t r(0); r != C.dimy; r++)
  {
    for(size_t c(0); c != C.dimx; c++)
    {
      for(auto& d : Cs.data)
      {
        if(d[0] == c && d[1] == r)
        {
          if(d.val != C[r][c])
          {
            std::cerr << "ERROR: Cs[" << r << ", " << c << "](" << d.val << ") != C[" << r << ", " << c << "](" << C[r][c] << ")\n";
            failed = true;
          }
        }
      }
    }
  }
  if(failed)
    std::cout << "test failed\n";
  else
    std::cout << "test passed\n";
  return failed ? -1 : 0;
}
