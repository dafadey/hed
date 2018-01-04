#include <iostream>
#include <string>
#include "zstream.h"

int main()
{
	gzostream zs("zstream_test_1");
	for(int i=0;i<256*1024;i++)
		zs << i << "\n";
	zs << "finish";
	zs.close();
	zs.open("zstream_test_2");
	zs << "new data" << "\n";
	for(int i=0;i<256*1025;i++)
		zs << -i << "\n";
	zs.close();
	zs << "out of stream record. this will be lost" << "\n";
	zs.open("zstream_test_3");
	zs << "brand new data" << "\n";
	std::cout << zs.size() << std::endl;

	gzistream izs("zstream_test_2");

	std::ofstream f("test.txt");
	std::string s;
	while(!izs.eof())
	{
		std::getline(izs,s);
		//std::cout << s << std::endl;
		f << s << std::endl;
	}
	f.close();
	
	return 0;
}
