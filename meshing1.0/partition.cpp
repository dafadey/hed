#include "partition.h"
#include <set>
#include <map>
#include <algorithm>

struct center_node
{
  center_node(const XY& xy, const ITRIANGLE* t) : coords(xy), tri(t) {}
  XY coords;
  const ITRIANGLE* tri;
};

void static remove_hanging(std::vector<std::set<const ITRIANGLE*>>& pre_output)
{
  // rebuild part_map
  std::map<const ITRIANGLE*, size_t> part_map;
  
  for(size_t part_id(0); part_id != pre_output.size(); part_id++)
  {
    const auto& part = pre_output[part_id];
    for(const ITRIANGLE* t : part)
      part_map[t] = part_id;
  }
  
  for(size_t part_id(0); part_id != pre_output.size(); part_id++)
  {
    auto& part = pre_output[part_id];
    std::vector<std::pair<const ITRIANGLE*,size_t>> destinations;
    for(auto t : part)
    {
      std::vector<std::pair<size_t,size_t>> inc_map;
      for(auto nbr : t->get_neighbors())
      {
        bool found(false);
        for(auto& it : inc_map)
        {
          if(part_map[nbr]==it.first)
          {
            it.second++;
            found=true;
          }
        }
        if(!found)
          inc_map.push_back(std::make_pair(part_map[nbr], 1));
      }
      std::sort(inc_map.begin(), inc_map.end(), [](const std::pair<size_t,size_t>& a, const std::pair<size_t,size_t>& b){ return a.second > b.second;});
      if(inc_map[0].first != part_id && inc_map[0].second > 1)
        destinations.push_back(std::make_pair(t, inc_map[0].first));
    }
    for(const auto& dest : destinations)
    {
      part.erase(dest.first);
      pre_output[dest.second].insert(dest.first);
      part_map[dest.first] = dest.second;
    }
  }
}

//KD partitioning

static std::vector<center_node*> delam(std::set<center_node*> part, int dir = 0)
{
  std::vector<std::pair<double, center_node*>> coords;
  for(auto c_node_ptr : part)
  {
    coords.push_back(std::make_pair(dir==0 ? c_node_ptr->coords.x : c_node_ptr->coords.y, c_node_ptr));
  }
  std::sort(coords.begin(), coords.end(), [](const std::pair<double, center_node*>& f, const std::pair<double, center_node*>& s) { return f.first < s.first; });
  size_t i(0);
  std::vector<center_node*> output;
  for(; i != coords.size(); i++)
  {
    if(coords[i].first > 0.5 * (coords[0].first + coords[coords.size() - 1].first) )
      output.push_back(coords[i].second);
  }
  return output;
}

static void partition(std::vector<std::set<center_node*>>& parts)
{
  size_t n = parts.size();
  std::cout << "parts sizes are :";
  for(auto& p : parts)
    std::cout << " " << p.size();
  std::cout << '\n';
  
  for(size_t i = 0; i != n; i++)
  {
    double xmin = std::numeric_limits<double>::max();
    double xmax = -xmin;
    double ymin = xmin;
    double ymax = xmax;
    for(auto c : parts[i])
    {
      xmin = std::min(xmin, c->coords.x);
      xmax = std::max(xmax, c->coords.x);
      ymin = std::min(ymin, c->coords.y);
      ymax = std::max(ymax, c->coords.y);
    }
  
    std::vector<center_node*> right = delam(parts[i], xmax - xmin > ymax - ymin ? 0 : 1);
    if(right.size() == 0)
      continue;
    parts.push_back(std::set<center_node*>());
    for(auto r : right)
    {
      parts[i].erase(r);
      parts.back().insert(r);
    }
  }
  std::cout << "parts sizes are :";
  for(auto& p : parts)
    std::cout << " " << p.size();
  std::cout << '\n';
}

std::vector<std::vector<const ITRIANGLE*>> KDpartition(const mesh& im, int depth)
{
  std::vector<center_node> centers;
  //get centers
  for(auto t : im.triangles)
    centers.push_back(center_node(t->centroid(), t));
  std::vector<std::set<center_node*>> links;
  links.push_back(std::set<center_node*>());
  for(auto& c : centers)
    links.back().insert(&c);
  for(int i(0); i != depth; i++)
    partition(links);
  
  std::vector<std::set<const ITRIANGLE*>> pre_output(links.size());
  for(size_t i(0); i != links.size(); i++)
  {
    for(auto c : links[i])
      pre_output[i].insert(c->tri);
  }
  
  remove_hanging(pre_output);
  
  std::vector<std::vector<const ITRIANGLE*>> output(links.size());
  for(size_t i(0); i != pre_output.size(); i++)
  {
    for(auto c : pre_output[i])
      output[i].push_back(c);
  }
  return output;
}

// Greedy partitioning

static void advance(std::set<const ITRIANGLE*>& front, std::set<const ITRIANGLE*>& todo)
{
  std::set<const ITRIANGLE*> tris;
  for(const auto t : front)
  {
    for(const auto it : t->get_surrounding())
      tris.insert(it);
    todo.erase(t);
  }

  for(auto t : front)
    tris.erase(t);

  front.clear();
  
  for(auto t : tris)
  {
    if(todo.find(t) != todo.end())
      front.insert(t);
  }
}

static std::set<const ITRIANGLE*> get_outers(const std::set<const ITRIANGLE*>& todo, bool bndry = true)
{
  std::set<const ITRIANGLE*> output;
  for(const ITRIANGLE* t : todo)
  {
    bool outer(false);

    for(const ITRIANGLE* nbrt : t->get_neighbors())
    {
      if(bndry && !nbrt)
        outer = true;
      
      if(todo.find(nbrt) == todo.end())
        outer = true;
    }
   
    if(outer)
      output.insert(t);
  }
  return output;
}

static std::set<const ITRIANGLE*> get_outers(const std::vector<const ITRIANGLE*>& todo, bool bndry = true)
{
  std::set<const ITRIANGLE*> input;
  for(auto t : todo)
    input.insert(t);
  return get_outers(input);
}

std::vector<std::vector<const ITRIANGLE*>> greedy_partition(const mesh& im, int part_size)
{
  std::set<const ITRIANGLE*> todo;
  std::set<const ITRIANGLE*> front;
  for(auto t : im.triangles)
    todo.insert(t);

  std::vector<std::set<const ITRIANGLE*>> pre_output;
  // go go go
  while(todo.size()>0)
  {
    pre_output.push_back(std::set<const ITRIANGLE*>());
    front.clear();
    
    //front.insert(*get_outers(todo, todo.size() == im.triangles.size()).begin());
    //choose the best first node. (at the most sharp corner)
    std::set<const ITRIANGLE*> outers = get_outers(todo, todo.size() == im.triangles.size());
    size_t surr_sz = std::numeric_limits<size_t>::max();
    const ITRIANGLE* starting_tri = nullptr;
    for(const ITRIANGLE* t : outers)
    {
      if(t->get_surrounding().size() < surr_sz)
      {
        surr_sz = t->get_surrounding().size();
        starting_tri = t;
      }
    }
    front.insert(starting_tri);
    
    size_t done_local(front.size());
   
    while(todo.size()>0 && done_local < part_size)
    {
      for(const ITRIANGLE* f : front)
        pre_output.back().insert(f);
      advance(front, todo);
      if(front.size() == 0)
        break;
      done_local += front.size();
    }
  }
  //refine
  // remove small partitions - join with closest smaller partition (need threshold - HEURISTICS!!!)
  std::vector<size_t> parts_to_join_ids;
  std::map<const ITRIANGLE*, size_t> part_map;
  for(size_t part_id(0); part_id != pre_output.size(); part_id++)
  {
    const auto& part = pre_output[part_id];
    for(const ITRIANGLE* t : part)
      part_map[t] = part_id;
  }

  std::map<size_t, std::set<size_t>> part_nbr_map;
  for(size_t part_id(0); part_id != pre_output.size(); part_id++)
  {
    part_nbr_map[part_id] = std::set<size_t>();
    auto outers = get_outers(pre_output[part_id]);
    for(auto t : outers)
    {
      for(auto nbr : t->get_neighbors())
      {
        if(nbr)
        {
          if(part_map[nbr] != part_id)
            part_nbr_map[part_id].insert(part_map[nbr]);
        }
      }
    }
    const auto& part = pre_output[part_id];
  }
  
  for(size_t part_id(0); part_id != pre_output.size(); part_id++)
  {
    if(pre_output[part_id].size() * 10 < part_size)
      parts_to_join_ids.push_back(part_id);
  }
  
  for(const auto& small_part_id : parts_to_join_ids)
  {
    size_t minimal=std::numeric_limits<size_t>::max();
    size_t minimal_id=0;
    for(size_t nbr_part_ids : part_nbr_map[small_part_id])
    {
      if(pre_output[nbr_part_ids].size() < minimal)
      {
        minimal = pre_output[nbr_part_ids].size();
        minimal_id = nbr_part_ids;
      }
    }
    for(auto t : pre_output[small_part_id])
      pre_output[minimal_id].insert(t);
    pre_output[small_part_id].clear();
  }

  for(ssize_t part_id(pre_output.size() - 1); part_id > -1; part_id--)
  {
    if(pre_output[part_id].size()==0)
      pre_output.erase(pre_output.begin()+part_id);
  }

  // remove hanging triangles
  remove_hanging(pre_output);
  
  // reformat result for output
  std::vector<std::vector<const ITRIANGLE*>> output(pre_output.size());
  for(size_t part_id(0); part_id != pre_output.size(); part_id++)
  {
    const auto& part = pre_output[part_id];
    for(auto t : part)
      output[part_id].push_back(t);
  }
  
  return output;
}
