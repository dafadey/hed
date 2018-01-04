#include <pthread.h>
#include <string>
#include <map>
#include "soc.h"

struct fadey_server;

fadey_server* create_fadey_server();

struct fadey_server
{
	friend fadey_server* create_fadey_server();
	static pthread_mutex_t map_access_mutex;
	static bool map_access_mutex_inited;
	static std::map<pthread_t, fadey_server*> servers;
	std::map<pthread_t, soc> clients; // all clients connected
	std::string ip;
	int port;
	soc s; // server listening socket
	soc last_client; // last connected client
	pthread_t server_thread;
	pthread_mutex_t server_mutex;

	void (*server_func_ptr)(int);
	
	bool start();
	
	void stop();
	
	static void *listener_func(void* arg);
	
	static void *client_func(void* arg);
	
	void (*server_func)();

	static void send(const std::string& s);
	static void recv(std::string& s);

	private:
	fadey_server(const char* _ip=soc::defIP, int _port=defPORT) : 
	             clients(), ip(_ip), port(_port), s(), last_client(), server_thread(), server_mutex() {}
	
};
