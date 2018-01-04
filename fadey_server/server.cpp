#include "server.h"
#include <iostream>

std::map<pthread_t, fadey_server*> fadey_server::servers;
pthread_mutex_t fadey_server::map_access_mutex;
bool fadey_server::map_access_mutex_inited(false);


fadey_server* create_fadey_server()
{
	fadey_server* s = new fadey_server;
	if(!fadey_server::map_access_mutex_inited)
	{
		pthread_mutex_init(&fadey_server::map_access_mutex, 0);
		fadey_server::map_access_mutex_inited = true;
	}
	return s;
}

bool fadey_server::start()
{
	pthread_mutex_init(&server_mutex, 0);
	pthread_mutex_lock(&server_mutex);
	s.srv_bind(ip.c_str(), port);
	pthread_create(&server_thread, 0, listener_func, this);
}

void fadey_server::stop()
{
	s.Close();
}

void *fadey_server::listener_func(void* arg)
{
	fadey_server* s_ptr = (fadey_server*) arg;
	while(1)
	{
		soc cl = s_ptr->s.listen_new_client();
		if(s_ptr->s.sh < 0 || cl.sh < 0)
			break;
		s_ptr->last_client = cl;
		pthread_t cl_thread_id;
		pthread_create(&cl_thread_id, NULL, client_func, s_ptr);
		pthread_mutex_lock(&(s_ptr->server_mutex)); // this can be a bit laggy...
	}
}

void *fadey_server::client_func(void* arg)
{
	
	fadey_server* s_ptr = (fadey_server*) arg;
	soc cl = s_ptr->last_client;
	pthread_t this_thread = pthread_self();
	pthread_mutex_lock(&fadey_server::map_access_mutex);
	s_ptr->clients.insert(std::make_pair(this_thread, cl));
	fadey_server::servers.insert(std::make_pair(this_thread, s_ptr));
	pthread_mutex_unlock(&fadey_server::map_access_mutex);
  pthread_mutex_unlock(&(s_ptr->server_mutex));
	s_ptr->server_func();
	cl.Close();
	
	//lock own map access mutex and remove map value.
	pthread_mutex_lock(&fadey_server::map_access_mutex);
	s_ptr->clients.erase(this_thread);
	fadey_server::servers.erase(this_thread);
	pthread_mutex_unlock(&fadey_server::map_access_mutex);
}

void fadey_server::recv(std::string& s)
{
	//lock map access mutex before manipulating the map
	pthread_mutex_lock(&fadey_server::map_access_mutex);
	pthread_t this_thread = pthread_self();
	fadey_server* s_ptr = fadey_server::servers[this_thread];
	soc cl = s_ptr->clients[this_thread];
	//unlock mutex
	pthread_mutex_unlock(&fadey_server::map_access_mutex);
	soc::Read(cl, s);
}

void fadey_server::send(const std::string& s)
{
	//lock map access mutex before manipulating the map
	pthread_mutex_lock(&fadey_server::map_access_mutex);
	pthread_t this_thread = pthread_self();
	fadey_server* s_ptr = fadey_server::servers[this_thread];
	soc cl = s_ptr->clients[this_thread];
	//unlock mutex
	pthread_mutex_unlock(&fadey_server::map_access_mutex);
	soc::Send(cl, s);
}
