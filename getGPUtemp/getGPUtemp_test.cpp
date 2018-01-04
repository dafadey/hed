#include "getGPUtemp.h"
#include <cstdlib>
#include <cstdio>

int main(int argc, char** argv)
{
	int n;
	if(argc==1)
		n=0;
	else
		n=atoi(argv[1]);
	printf("gpu #%d temp is %d\n",n,getGPUtemp(n));
	return 0;
}
