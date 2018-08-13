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

    return P * A * P_1;
  }
  
  template <typename T>
  sparse_matrix<T> jacobi_diag(const sparse_matrix<T>& A)
  {
    sparse_matrix<T> R;
    for(const auto& itm : A.data)
      R.add_item(itm[0], itm[1], itm.val);
    R.make_rows_and_cols();
    
    T maxdiag(0);
    for(const auto& itm : A.data)
    {
      if(itm[0] == itm[1])
        maxdiag = std::max(maxdiag, std::abs(itm.val));
    }
    std::cout << "maxdiag = " << maxdiag << '\n';
    static int steps(0);
    while(true)
    {
      size_t col(0);
      size_t row(0);
      T val(0);
      for(const auto& itm : R.data)
      {
        if(itm[0] == itm[1])
          continue;
        //std::cout << std::abs(itm.val) << ", ";
        if(std::abs(itm.val) > val) // dev., note here you can meet difficulties with complex numbers
        {
          col = itm[0];
          row = itm[1];
          val = std::abs(itm.val);
        }
      }
      //std::cout << '\n';
      //std::cout << "max non-diag=" << val << ", col=" << col << ", row=" << row << '\n';
      if(val < 1.e-8 * maxdiag)
      {
        std::cout << "number of steps is " << steps << '\n';
        return R;
      }
/*

      matrix<T> R_(R.dimx, R.dimy);
      for(const auto& it : R.data)
        R_.set(it[0], it[1], it.val);
      R_.dump("R");
*/
      auto r = jacobi_rotate(R, col, row);

/*
      matrix<T> r_(r.dimx, r.dimy);
      for(const auto& it : r.data)
        r_.set(it[0], it[1], it.val);
      r_.dump("r");
*/

      R.clear();
      for(const auto& itm : r.data)
        R.add_item(itm[0], itm[1], itm.val);
      R.make_rows_and_cols();
      steps++;
      //if(steps==3333)
      //  return R;
      
    }
  }

}//namespace
