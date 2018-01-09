#pragma once
#include <mesh.h>
#include "postmeshing.h"

struct hed_data
{
	mesh* m;
	weights* w;
	
	std::vector<hed_data_type> e;
	std::vector<hed_data_type> h;
	std::vector<hed_data_type> j;
	std::vector<hed_data_type> n;

	hed_data_type dt;
	
	void calc_e();
	void calc_h();
	//void calc_j();
	//void calc_h();
	
};

