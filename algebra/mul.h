/* templates for algebra with sparse matrices
 * fadey */
#pragma once

#include "sparse.h"
#include "matrix.h"

namespace algebra
{
  template <typename T>
  sparse_matrix<T> operator*(sparse_matrix<T>& a, sparse_matrix<T>& b)
  {
    sparse_matrix<T> res;
    if(!a.coherent)
      a.make_rows_and_cols();
    if(!b.coherent)
      b.make_rows_and_cols();
    //multiplication
    for(const auto& bcol : b.cols)
    {
      if(bcol.size() == 0)
        continue;
      for(const auto& arow : a.rows)
      {
        if(arow.size() == 0)
          continue;
        size_t a_idx(0);
        size_t b_idx(0);
        T _res(0);
        while(true)
        {
          size_t a_id = arow[a_idx].id;
          size_t b_id = bcol[b_idx].id;
          if(a_id < b_id)
            a_idx++;
          else if(a_id > b_id)
            b_idx++;
          else /* == */
          {
            _res += arow[a_idx].val * bcol[b_idx].val;
            a_idx++;
            b_idx++;
          }
          
          //std::cout << a_idx << ", " << b_idx << " limits are: " << arow.size() << ", " << bcol.size() << '\n';
          if(a_idx == arow.size() || b_idx == bcol.size())
            break;
        }
        res.add_item(bcol.id, arow.id, _res);
      }
    }
    res.make_rows_and_cols();
    return res;
  }

  template <typename T>
  matrix<T> operator*(const matrix<T>& a, const matrix<T>& b)
  {
    if(a.dimx != b.dimy)
      return matrix<T>(0, 0);
    size_t dim = a.dimx;
    matrix<T> res(b.dimx, a.dimy);
    //multiplication
    for(size_t r(0); r != res.dimy; r++)
    {
      for(size_t c(0); c != res.dimx; c++)
      {
        T _res(0);
        for(size_t d(0); d != dim; d++)
          _res += a.get(d, r) * b.get(c, d);
        res.set(c, r, _res);
      }
    }
    return res;
  }
}
