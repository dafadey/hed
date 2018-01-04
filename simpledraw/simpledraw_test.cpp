#include "simpledraw.h"
#include <iostream>
#include <string>
#include <cmath>
#include <unistd.h>
#define FL_DBL float//double

void add_droplett(FL_DBL* data, int Nx, int Ny, FL_DBL x=0.5f, FL_DBL y=0.5f, FL_DBL sz=0.1, FL_DBL amp=0.5)
{
	for(int i=0;i!=Ny;i++)
	{
		for(int j=0;j!=Nx;j++)
		{
			FL_DBL xx = FL_DBL(j) / FL_DBL(Nx);
			FL_DBL yy = FL_DBL(i) / FL_DBL(Ny);
			data[i * Nx + j] += amp * exp(-pow((xx-x)/sz, 2.0)-pow((yy-y)/sz, 2.0));
		}
	}
}

int main()
{
	int Nx=127;
	int Ny=129;
	fadey_init(Nx,Ny,2);
	FL_DBL* data = new FL_DBL[Nx * Ny];
	for(int i=0;i<Nx*Ny;i++)
		data[i] = 0.0;
	add_droplett(data,Nx,Ny);
	std::cout << "data is filled" << std::endl;
	fadey_draw(data,Nx,Ny,0);
	std::cout << "draw complete" << std::endl;
	FL_DBL a=1.0f;
	//sleep(10);
	std::cout << "sleep complete" << std::endl;
	while(a!=0.0f)
	{
		FL_DBL x;
		std::cout << "set droplet x[0..1]:";
		std::cin >> x;
		FL_DBL y;
		std::cout << "set droplet y[0..1]:";
		std::cin >> y;
		FL_DBL s;
		std::cout << "set size [0....]";
		std::cin >> s;
		std::cout << "set droplet amplitude(set 0 to finish):";
		std::cin >> a;
		add_droplett(data,Nx,Ny,x,y,s,a);
		fadey_draw(data,Nx,Ny,1);
	}
	fadey_close();
	delete[] data;
}
