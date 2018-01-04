#include <iostream>
#include <soc.h>
#include "tclient.h"
#include <unistd.h>

double getSensor(const std::string& query)
{
	soc s;
	if(!s.client_start())
	{
		return -1.0;
	}
	soc::Send(s, query);
	std::string responce;
	soc::Read(s, responce);
	s.Close();
	if(responce!="no such sensor\n")
		return atof(responce.c_str());
	else
		return 0.0;
}

int devicecool(int i)
{
	if(i < 0 || i > 7)
		return true;
	const double t_chip = getSensor("PRI_RD890S_THERM");
	const double t_card = getSensor("GPU"+std::to_string(i));
	if(t_chip == -1.0 || t_card == -1.0)
		return -2;
	if(t_chip < 83 && t_card < 80)
		return 0;
	std::cerr << "device " << i << " T=" << t_card << "C" << ", chip T=" << t_chip << "C" << std::endl;
	return -1;
}

bool cooldevice(int i)
{
	while(true)
	{
		int d=devicecool(i);
		if(d==0)
			return true;
		else if(d==-2)
			return false;
		sleep(7);
	}
}
