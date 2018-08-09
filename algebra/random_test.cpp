#include <iostream>
#include "mul.h"
#include "sparse.h"
#include "matrix.h"
#include <ctime>
#include <cmath>

double unitrand()
{
  return (double) rand() / (double) RAND_MAX;
}

int main(int argc, char* argv[])
{
  int seed = (int) time(NULL);
  std::cout << "seed is " << seed << '\n';
  srand(seed);
  size_t Nx = 513;
  size_t Ny = 1023;
  if(argc>1)
    Nx=atoi(argv[1]);
  if(argc>2)
    Ny=atoi(argv[2]);
    
  std::cout << "setting regular matrixes...\n";
  algebra::matrix<double> A(Nx, Ny);
  algebra::matrix<double> B(Ny, Nx);
  algebra::sparse_matrix<double> As;
  algebra::sparse_matrix<double> Bs;
  for(int i(0); i < 13 * sqrt(Nx * Ny); i++)
  {
    size_t col = rand() % Nx;
    size_t row = rand() % Ny;
    double val = unitrand()*0.9 + 0.1;
    A.set(col, row, val);
    col = rand() % Ny;
    row = rand() % Nx;
    val = unitrand()*0.9 + 0.1;
    B.set(col, row, val);
  }

  std::cout << "setting sparse matrixes...\n";

  for(int r=0; r!=A.dimy; r++)
  {
    for(int c=0; c!=A.dimx; c++)
    {
      if(A.get(c, r) != 0)
        As.add_item(c, r, A.get(c, r));
    }
  }

  for(int r=0; r!=B.dimy; r++)
  {
    for(int c=0; c!=B.dimx; c++)
    {
      if(B.get(c, r) != 0)
        Bs.add_item(c, r, B.get(c, r));
    }
  }

  
  std::cout << "regular multiplication...\n";
  auto C = A * B;
  std::cout << "\tdone\n";
  std::cout << "sparse multiplication...\n";
  auto Cs = As * Bs;
  std::cout << "\tdone\n";
  //As.dump();
  //Bs.dump();
  std::cout << "computation is done, running test...\n";
  
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
