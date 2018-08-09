/* templates for algebra with sparse matrices
 * fadey */
#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>

namespace algebra
{
template <typename T>
struct sparse_matrix
{
  // basic data and functions
  struct item
  {
    item(size_t cid, size_t rid, const T& value) : col_id(cid), row_id(rid), val(value) {}
    size_t col_id;
    size_t row_id;
    size_t operator[](size_t i) const
    {
      return i == 0 ? col_id : row_id;
    }
    T val;
  };
  
  std::vector<item> data;

  void add_item(size_t row_id, size_t col_id, const T& value)
  {
    data.emplace_back(item(row_id, col_id, value));
    coherent = false;
  }

  T get(size_t col, size_t row)
  {
    if(!coherent)
    {
      for(const auto& i : data)
      {
        if(i[0] != col || i[1] != row)
          continue;
        return i.val;
      }
      return T(0);
    }
    else
    {
      auto itr = std::lower_bound(rows.begin(), rows.end(), line(row), [](const line& l0, const line& l1) {return l0.id < l1.id;});
      if(itr == rows.end() || itr->id != row)
        return T(0);
      auto& r = *itr;
      auto itc = std::lower_bound(r.begin(), r.end(), item_reduced(col), [](const item_reduced& ir0, const item_reduced& ir1) {return ir0.id < ir1.id;});
      if(itc == r.end() || itc->id != col)
        return T(0);
      return itc->val;
    }
  }

  // helpers
  bool coherent; // means cache structures has all data
  
  struct item_reduced
  {
    item_reduced(size_t _id) : id(_id), val(0) {}
    item_reduced(size_t _id, const T& value) : id(_id), val(value) {}
    item_reduced(const item& itm, int coord /*0 for column, 1 for row*/) : id(itm[coord]), val(itm.val) {}
    size_t id;
    T val;
  };
  
  struct line : public std::vector<item_reduced>
  {
    line(size_t _id) : std::vector<item_reduced>(), id(_id) {};
    size_t id;
  };

  std::vector<line> rows;
  std::vector<line> cols;

  void make_rows_and_cols()
  {
    size_t dim[2] {0, 0};
    for(int i(0); i != 2; i++)
      dim[i] = (*std::max_element(data.begin(), data.end(), [i](const item& a, const item& b) {return a[i] < b[i];}))[i] + 1;
    std::array<std::vector<size_t>, 2> lookup; // we have dim[0] of cols and dim[1] of rows
    for(int i(0); i != 2; i++)
      lookup[i].resize(dim[i], std::numeric_limits<size_t>::max());
    std::vector<line>* lines_ptr[2]{&cols, &rows};
    for(const auto& itm : data)
    {
      for(int i(0); i != 2; i++)
      {
        if(lookup[i][itm[i]] == std::numeric_limits<size_t>::max()) // have this line already
        {
          lookup[i][itm[i]] = lines_ptr[i]->size();
          lines_ptr[i]->push_back(line(itm[i]));
        }
        (*lines_ptr[i])[lookup[i][itm[i]]].push_back(item_reduced(itm, 1 - i));
      }
    }
    //canonicalize matrix cache values : order cols and rows
    for(int i(0); i != 2; i++)
    {
      sort(lines_ptr[i]->begin(), lines_ptr[i]->end(), [](const line& l0, const line& l1) { return l0.id < l1.id;});
      for(auto& l : *lines_ptr[i])
        sort(l.begin(), l.end(), [](const item_reduced& ir0, const item_reduced& ir1) { return ir0.id < ir1.id;});
    }
    coherent = true;
  }
  
  void dump(const std::string name = "", const std::string indent = "")
  {
    std::cout << name << '\n';
    if(!coherent)
    {
      std::cout << indent;
      for(const auto& it : data)
        std::cout << " [" << it[0] << ", " << it[1] << "]:" << it.val;
      std::cout << "\ndo make_rows_and_cols() for your matrix if you want to see rows and columns\n";
    }
    else
    {
      std::vector<line>* lines_ptr[2] {&cols, &rows};
      for(int i(0); i != 2; i++)
      {
        std::cout << indent << (i==0 ? "cols:" : "rows:") << '\n';
        for(const auto& ln : *lines_ptr[i])
        {
          std::cout << indent << ln.id << ":";
          for(const auto& itmr : ln)
            std::cout << " [" << itmr.id << "]" << itmr.val;
          std::cout << '\n';
        }
      }
    }
  }

};
}//namespace
