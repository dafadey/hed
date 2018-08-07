/* templates for algebra with regular matrices
 * fadey */
#pragma once
#include <vector>
#include <iostream>

namespace algebra
{
template <typename T>
struct matrix : public std::vector<std::vector<T>>
{
  matrix(size_t _dimx, size_t _dimy) : dimx(_dimx), dimy(_dimy), std::vector<std::vector<T>>()
  {
    for(size_t c(0); c != dimy; c++)
    {
      this->push_back(std::vector<T>());
      this->back().resize(dimx);
    }
  }
  size_t dimx;
  size_t dimy;
  void set(size_t col, size_t row, const T& v)
  {
    (*this)[row][col] = v;
  }
  
  T get(size_t col, size_t row) const
  {
    return (*this)[row][col];
  }
  
  void dump(const std::string name = "", const std::string indent = "")
  {
    std::cout << name << '\n';
    for(size_t r(0); r != dimy; r++)
    {
      std::cout << indent;
      for(size_t c(0); c != dimx; c++)
        std::cout << " " << get(c, r);
      std::cout << '\n';
    }
  }
};
};//namespace
