/* templates for algebra with sparse matrices
 * fadey */
#pragma once

#include "mul.h"
#include <cmath>

namespace algebra
{
  template <typename T>
  sparse_matrix<T> jacobi_rotate(sparse_matrix<T>& A, size_t col, size_t row)
  {
    if(col == row)
      return A;
    
    if(!A.coherent)
      A.make_rows_and_cols();
    
    sparse_matrix<T> P;
    sparse_matrix<T> P_1;
    T a = A.get(col,col);
    T b = A.get(row,row);
    T d = A.get(col,row);
    T x = d * d / ((a - b) * (a - b) + 4.0 * d * d);
    T c = sqrt(.5 + sqrt(.25 - x));
    T s = sqrt(.5 - sqrt(.25 - x));
    
    for(size_t i(0); i!= A.dimx; i++)
    {
      if( i == col || i == row )
      {
        P.add_item(i, i, c);
        P_1.add_item(i, i, c);
        continue;
      }
      P.add_item(i, i, T(1));
      P_1.add_item(i, i, T(1));
    }
    
    P.add_item(col, row , s);
    P_1.add_item(col, row , -s);
    P.add_item(row, col , -s);
    P_1.add_item(row, col , s);
    P.make_rows_and_cols();
    P_1.make_rows_and_cols();

    std::cout << "1\n";
    return P * A * P_1;
  }
  
  /*
  template <typename T>
  sparse_matrix<T> diag(const sparse_matrix<T>& a, size_t col, size_t row)
  {
  }
  */
  

}//namespace
