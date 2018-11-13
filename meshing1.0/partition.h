#pragma once
#include "mesh.h"
#include "elements.h"
#include <vector>

std::vector<std::vector<const ITRIANGLE*>> KDpartition(const mesh& im, int depth);
std::vector<std::vector<const ITRIANGLE*>> greedy_partition(const mesh& im, int part_size);
