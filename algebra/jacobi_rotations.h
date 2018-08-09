/* templates for algebra with sparse matrices
 * fadey */
#pragma once

#include "mul.h"

namespace algebra
{
  template <typename T>
  sparse_matrix<T> jacobi_rotate(sparse_matrix<T>& a, size_t col, size_t row)
  {
    if(col == row)
      return a;
    
    if(!a.coherent)
      a.make_rows_and_cols();
    
    sparse_matrix<T> p;
    sparse_matrix<T> p_1;
    T arg = a.get(col,row);
    T cs;
    T sn;
    
    for(size_t i(0); i!= a.dimx; i++)
    {
      if( i == col || i == row )
      {
        p.add_item(i, i, cs);
        p_1.add_item(i, i, cs);
        continue;
      }
      p.add_item(i, i, T(1));
      p_1.add_item(i, i, T(1));
    }
    
    p.add_item(col, row , sn);
    p_1.add_item(col, row , -sn);
    p.add_item(row, col , -sn);
    p_1.add_item(row, col , sn);
    
    return p * a * p_1;
  }
  
  /*
  template <typename T>
  sparse_matrix<T> diag(const sparse_matrix<T>& a, size_t col, size_t row)
  {
  }
  */
  

}//namespace
