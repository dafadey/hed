/* templates for algebra with sparse matrices
 * fadey */
#pragma once

#include "sparse.h"
#include "matrix.h"

namespace algebra
{
  template <typename T>
  sparse_matrix<T> operator*(const sparse_matrix<T>& a, const sparse_matrix<T>& b)
  {
    sparse_matrix<T> res;
    //if(!a.coherent)
    //  a.make_rows_and_cols();
    //if(!b.coherent)
    //  b.make_rows_and_cols();
    if(!a.coherent || !b.coherent)
    {
      std::cerr << "algebra::multiplication error. please make your matrices coherent. run meke_rows_and_cols() for each.\n";
      return res;
    }
    //multiplication
    std::vector<T> _res;
    _res.resize(std::min(b.cols.size(), a.rows.size()));
    for(const auto& bcol : b.cols)
    {
      if(bcol.size() == 0)
        continue;
      for(const auto& arow : a.rows)
      {
        for(auto& it : _res)
          it = T(0);
        if(arow.size() == 0)
          continue;
        size_t a_idx(0);
        size_t b_idx(0);
        T __res(0);
        size_t muls(0);
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
            _res[muls] = arow[a_idx].val * bcol[b_idx].val;
            a_idx++;
            b_idx++;
            muls++;
          }
          
          //std::cout << a_idx << ", " << b_idx << " limits are: " << arow.size() << ", " << bcol.size() << '\n';
          if(a_idx == arow.size() || b_idx == bcol.size())
            break;
        }
        if(!muls)
          continue;
        sort(_res.begin(), _res.end(), [](const T& v0, const T& v1) {return v0 > v1;});
        for(const auto& it : _res)
          __res += it;
        if(__res == T(0))
          continue;
        res.add_item(bcol.id, arow.id, __res);
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
}//namespace
