#include "tclient.h"
#include <string>
#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
	std::cout << "Hi" << std::endl;
	if(argc<2)
	{
		std::cout << "argc=" << argc << ", which is wrong please provide at least 1 argument" << std::endl;
		return -1;
	}
	std::vector<std::pair<std::string, double>> data;
	for(int i=1; i!=argc; i++)
	{
		std::string s(argv[i]);
		std::cout << "reading sensor " << s << std::endl;
		data.emplace_back(std::make_pair(s,getSensor(s)));
	}
	for(const auto& it : data)
		std::cout << it.first << " : " << it.second << std::endl;
	std::cout << "bye!" << std::endl;
	return 0;
}
