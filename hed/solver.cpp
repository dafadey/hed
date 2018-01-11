#include <iostream>
#include <map>
#include "solver.h"

void hed_data::calc_e()
{
	for(int i(0); i != e.size(); i++)
	{
		const ssize_t eid = i; // = m.edges[i].id;
		const ssize_t ir = m->edges[i]->t1 ? m->edges[i]->t1->id : -1;
		const ssize_t il = m->edges[i]->t2 ? m->edges[i]->t2->id : -1;
		e[i] -= ((ir == -1 ? hed_data_type(.0) : h[ir]) -
		         (il == -1 ? hed_data_type(.0) : h[il])) * w->edgs[i][1] * dt;
	}
}

void hed_data::calc_h()
{
	for(int i(0); i != h.size(); i++)
	{
		const ssize_t tid = i; // = m.triangles[i].id;
		hed_data_type db(hed_data_type(.0));
		for(size_t eid_local(0); eid_local != 3; eid_local++)
		{
			const size_t eid = m->triangles[i]->edges[eid]->id;
			db += (m->edges[eid]->t1 == m->triangles[tid] ? hed_data_type(1.0) : hed_data_type(-1.0))
             * e[eid] * w->edgs[eid][0]; // keep same orientation of edge as triangle have
		}
		h[i] += db * dt * w->tris[i][0];
	}
}

void hed_data::extract_contour_edges()
{
  std::map<IEDGE*, std::array<int, 2>> emap;
  int c_id_max(0);
  for(size_t n_id(0); n_id != m->contour_links.size(); n_id++)
  {
    int c_id = m->contour_links[c_id];
    c_id_max = std::max(c_id_max, c_id);
    for(auto e : m->nodes[n_id]->edges)
    {
      auto it = emap.find(e);
      if(it == emap.end())
        emap[e] = std::array<int, 2>{{0, c_id}};
      else
        it->second[0]++;
    }
  }
  contour_edges.resize(c_id_max);
  for(const auto& it : emap)
    contour_edges[it.second[1]].push_back(it.first->id);
}
