#include <iostream>
#include "solver.h"

void hed_data::calc_e()
{
	for(int i(0); i != e.size(); i++)
	{
		const ssize_t eid = i; // = m.edges[i].id;
		const ssize_t ir = m->edges[i]->t1 ? m->edges[i]->t1->id : -1;
		const ssize_t il = m->edges[i]->t2 ? m->edges[i]->t2->id : -1;
		e[i] += ((ir == -1 ? hed_data_type(.0) : h[ir]) -
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
			db += e[eid] * w->edgs[eid][0];
		}
		h[i] -= db * dt * w->tris[i][0];
	}
}
