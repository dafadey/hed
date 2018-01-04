#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#define SOCKET_ERROR -1
#define INVALID_SOCKET -1

#include <errno.h>
#include <stdio.h>
#include "soc.h"
#include <iostream>

const char* soc::defIP = "127.0.0.1";

bool operator<(const soc& a, const soc& b)
{
	return a.sh < b.sh;
}

bool soc::client_start(const char* net_addr, const int port)
{
	socklen_t addr_len = sizeof(sockaddr_in);
	sh = socket(AF_INET,SOCK_STREAM,0);

	if (sh < 0)
	{
		printf("socket %s create BAD with err ex code: %d \n",net_addr,sh);
		return false;
	}

	sockaddr_in addr;
	
	addr.sin_family = AF_INET;                                                                                         
	addr.sin_port = htons(port);                                                                                
	//gets inet ip addres
	struct hostent* hostinfo(gethostbyname(net_addr));
	if (!hostinfo)
	{
		printf("can't identify this address \n");
		return false;
	}
	
	addr.sin_addr = *((struct in_addr *) hostinfo->h_addr);
	int err_code = connect(sh, (struct sockaddr *) &addr, sizeof(addr));
	if (err_code<0)
	{
		printf("socket %s connect BAD with err ex code: %d \n",net_addr,err_code);
		return false;
	}
	//printf("socket %s connected OK \n",net_addr);
	return true;
}

// you can just bind the socket and do listen_new_client for any new connected client socket.
// this alows to make multi client server using pthreads
bool soc::srv_bind(const char* net_addr, const int port)
{
	socklen_t addr_len = sizeof(sockaddr_in);
	sh = socket(AF_INET,SOCK_STREAM,0);
	{
		int res;
		if(setsockopt(sh, SOL_SOCKET, SO_REUSEADDR, &res, sizeof(int)) < 0)
			printf("setsocket option SO_REUSEADDR failed with err code %d.\n", res);
	}
	if(sh < 0)
	{
		printf("socket %s create BAD with err ex code: %d \n",net_addr,sh);
		return false;
	}
	
	sockaddr_in addr;
	
	addr.sin_family = AF_INET;                                                                                         
	addr.sin_addr.s_addr = INADDR_ANY;                                                                                         
	addr.sin_port = htons(port);                                                                                
	//gets inet ip addres
	struct hostent* hostinfo(gethostbyname(net_addr));
	if (hostinfo == NULL)
	{
		printf("can't identify this address \n");
		return false;
	}
	
	addr.sin_addr = *((struct in_addr *) hostinfo->h_addr);

	int err_code = bind(sh, (struct sockaddr *) &addr, sizeof (addr));                                     
	if (err_code<0)
	{
		printf("socket %s bind BAD with err ex code: %d \n",net_addr,err_code);
		return false;
	}
	
	err_code = listen(sh, 1);                                     
	if (err_code<0)
	{
		printf("socket %s listen BAD with err ex code: %d \n",err_code);
		return false;
	}
	return true;
}

soc soc::listen_new_client() //returns client's handler
{
	sockaddr_in cl_addr;
	socklen_t addr_len = sizeof(sockaddr_in);
	
	soc cl;
	
	cl.sh = accept(sh, (struct sockaddr *) &cl_addr, &addr_len);                                   
	
	if (cl.sh==INVALID_SOCKET)
		printf("socket %d connect BAD with err ex code: %d \n",sh,cl.sh);
	//else
	//	printf("socket %d connect to server: %d \n",cl.sh, sh);
	
	return cl;
}

bool soc::srv_start(const char* net_addr, const int port) //returns client's handler
{
	if(!srv_bind(net_addr, port))
	{
		printf("srv_bind error\n");
		return INVALID_SOCKET;
	}
	
	soc cl = listen_new_client();                                     
	
	if (cl.sh != INVALID_SOCKET)
	{
		printf("socket %d connected to server OK \n",sh);
		return true;
	}
	else
	{
		printf("socket %d connect BAD with err ex code: %d \n",sh,cl.sh);
		return false;
	}
}

void soc::Close()
{
	int err_code = close(sh);
	if (err_code!=0)
		printf("socket %d close BAD with err ex code: %d \n",sh,err_code);
	//else
	//	printf("socket %d close OK \n",sh);
	sh=-1;
}

#define BUFFSZ 512

void soc::Send(soc cl, const std::string&s)
{
	ssize_t status(0);
	size_t pos(0);
	//std::cout << "soc::Send\n";
	while(status = send(cl.sh, &s.c_str()[pos], (s.size() - pos) > BUFFSZ ? BUFFSZ : (s.size() - pos), 0) > 0)
	{
		if(status == 1)
			pos += (s.size() - pos) > BUFFSZ ? BUFFSZ : (s.size() - pos);
		if(pos==s.size())
			break;
	}
	//std::cout << "    soc::Send - done\n";
}

void soc::Read(soc cl, std::string& s)
{
	char buf[BUFFSZ];
	ssize_t actual;
	while(actual = read(cl.sh, buf, BUFFSZ))
	{
		//std::cout << "server got " << actual << " bytes of request" << std::endl;
		s += std::string(buf, actual);
		if(actual < BUFFSZ)
			break;
	}
}

#undef BUFFSZ
