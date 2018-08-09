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
  for(int r=0; r!=Ny; r++)
  {
    for(int c=0; c!=Nx; c++)
    {
      A.set(c, r, 0.0);
    }
  }
  algebra::sparse_matrix<double> As;
  for(int i(0); i < 13 * sqrt(Nx * Ny); i++)
  {
    size_t col = rand() % Nx;
    size_t row = rand() % Ny;
    double val = unitrand()*0.9 + 0.1;
    A.set(col, row, val);
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
  
  //std::cout << "making sparce coherent...\n";
  As.make_rows_and_cols();
  
  std::cout << "testing...\n";
  //the test
  bool failed(false);
  for(size_t r(0); r != A.dimy; r++)
  {
    for(size_t c(0); c != A.dimx; c++)
    {
      if(As.get(c, r) != A.get(c, r))
      {
        std::cerr << "ERROR: As[" << r << ", " << c << "](" << As.get(c, r) << ") != A[" << r << ", " << c << "](" << A.get(c, r) << ")\n";
        failed = true;
      }
    }
  }
  if(failed)
    std::cout << "test failed\n";
  else
    std::cout << "test passed\n";
  return failed ? -1 : 0;
}
