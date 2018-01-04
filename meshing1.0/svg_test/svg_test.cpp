#include "svg.h"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
  std::string filename(argv[1]);
	std::vector<point> nodes;
	int read_res = svg::import(filename, nodes, 12, 1);
	if(read_res==-1)
		std::cout << "no such contour" << std::endl;
	else if(read_res==0)
		std::cout << "contour is read" << std::endl;
	else if(read_res==1)
		std::cout << "path is read" << std::endl;
	for(auto node : nodes)
		std::cout << "(" << node.x << ", " << node.y << ")" << std::endl;
	std::ofstream outfile("out.dat");
	for(int i=0;i!=nodes.size();i++)
		outfile << "e" << " " << nodes[i].x << " " << nodes[i].y << " " << nodes[(i+1 != nodes.size()) ? i+1 : 0].x << " " << nodes[(i+1 != nodes .size()) ? i+1 : 0].y << std::endl;
	for(int i=0;i!=nodes.size();i++)
		outfile << "p" << " " << nodes[i].x << " " << nodes[i].y << std::endl;
	outfile.close();
	return 0;
}
