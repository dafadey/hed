#include "tclient.h"
#include <string>
#include <iostream>
#include <vector>
#include <unistd.h>

int main(int argc, char* argv[])
{
	std::cout << "Hi" << std::endl;
	if(argc<2)
	{
		std::cout << "argc=" << argc << ", which is wrong please provide sensor and number of times to read" << std::endl;
		return -1;
	}
	std::vector<std::pair<std::string, double>> data;
	for(int i=0; i!=atoi(argv[2]); i++)
	{
		std::string s(argv[1]);
		std::cout << getSensor(s) << "\n";
		usleep(100);
	}
	return 0;
}
