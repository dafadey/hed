#include <iostream>
#include <set>
#include "solver.h"

void hed_data::calc_e()
{
//  double avg_w(.0);
  for(int i(0); i != e.size(); i++)
  {
    const ssize_t eid = i; // = m.edges[i].id;
    const ssize_t ir = m->edges[eid]->t1 ? m->edges[eid]->t1->id : -1;
    const ssize_t il = m->edges[eid]->t2 ? m->edges[eid]->t2->id : -1;
//    avg_w += w->edgs[i][1];
    e[i] -= ((ir == -1 ? hed_data_type(.0) : h[ir]) -
             (il == -1 ? hed_data_type(.0) : h[il])) * w->edgs[i][1] * dt;
  }
//  std::cout << "avg w_e=" << avg_w / (double) e.size() << "\n";
}

void hed_data::calc_h()
{
  for(int i(0); i != h.size(); i++)
  {
    const ssize_t tid = i; // = m.triangles[i].id;
    hed_data_type db(hed_data_type(.0));
    for(size_t eid_local(0); eid_local != 3; eid_local++)
    {
      const size_t eid = m->triangles[tid]->edges[eid_local]->id;
      db += (m->edges[eid]->t1 == m->triangles[tid] ? hed_data_type(1.0) : hed_data_type(-1.0))
             * e[eid] * w->edgs[eid][0]; // keep same orientation of edge as triangle have
      //std::cout << w->tris[i][0] * w->edgs[eid][0] << "\n";
    }
    h[i] += db * dt * w->tris[i][0];
  }
}

void hed_data::extract_contour_edges()
{
  std::vector<IEDGE*> edg_map;
  int c_id_max(0);
  //std::cout << "countour links size is " << m->contour_links.size() << "\n";
  for(size_t e_id(0); e_id != m->fixed_edges.size(); e_id++)
  {
    int c_id = m->fixed_edges_mask[e_id];
    //std::cout << "doing contour # " << c_id << "\n";
    c_id_max = std::max(c_id_max, c_id);
    const auto& e = m->fixed_edges[e_id];
    std::set<IEDGE*> connected_edges;
    IEDGE* edg(nullptr);
    for(auto n : e)
    {
      for(auto ce : n->edges)
      {
        auto res = connected_edges.insert(ce);
        if(!res.second)
        {
          edg = ce;
          break;
        }
      }
    }
    if(edg)
      edg_map.push_back(edg);
  }
  if(edg_map.size() != m->fixed_edges.size())
  {
    std::cerr << "failed to collect fixed edges\n";
    return;
  }

  contour_edges.resize(c_id_max + 1);
  for(size_t e_id(0); e_id != edg_map.size(); e_id++)
  {
    const auto& e = edg_map[e_id];
    contour_edges[m->fixed_edges_mask[e_id]].push_back(e->id);
  }
}
