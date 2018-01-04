#include <iostream>
#include <nvml.h>
#include <unistd.h>
#include <sys/wait.h>
#include <server.h>

#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>

unsigned int getGPUtemp(int dev_id)
{
	nvmlDevice_t device;
	nvmlReturn_t result = nvmlDeviceGetHandleByIndex(dev_id, &device);
  if (result != NVML_SUCCESS)
  { 
    printf("Failed to get handle for device %i: %s\n", dev_id, nvmlErrorString(result));
    return 100;
  }

	unsigned int temp;
	if(nvmlDeviceGetTemperature(device,NVML_TEMPERATURE_GPU,&temp) == NVML_SUCCESS)
		return temp;
	else
		return 100;
}

std::map<std::string, double> data;

pthread_mutex_t trigger_mutex;

volatile int* quit;
volatile int* data_ready;

void reporter()
{
  std::cout << "#no of clients is : " << fadey_server::servers.begin()->second->clients.size() << std::endl;
	std::string got;
	fadey_server::recv(got);

  std::cout << "got: " << got << std::endl;
  if(got.c_str()[got.size()-1] == 10)
		got.pop_back();
  if(got.c_str()[got.size()-1] == 13)
		got.pop_back();
	if(got == "exit")
		*quit = 1;
	pthread_mutex_unlock(&trigger_mutex);
	while(!*data_ready);
  auto it = data.find(got);
  std::cout << "client asked for sensor \"" << got << "\"" << std::endl;
  std::string sent;
  if(it != data.end())
		sent = std::to_string(data[got])+"\n";
	else
		sent = "no such sensor\n";


	//sent = std::to_string(rand());
	fadey_server::send(sent);
}

int main(int argc, char* argv[])
{
	quit = new int;
	*quit = 0;
	data_ready = new int;
	*data_ready = 0;
	bool verbose(false);

	if(nvmlInit() != NVML_SUCCESS)
	{
		std::cerr << "Failed to initialize nvml library to control temperature of GPU boards" << std::endl;
		return -1;
	}

	pthread_mutex_init(&trigger_mutex, 0);
	pthread_mutex_lock(&trigger_mutex);

	fadey_server* s = create_fadey_server();
	s->server_func = &reporter;
	s->start();

	while(*quit == 0)
	{
		for(size_t i(0); i != 8; i++)
		{
			double t(getGPUtemp(i));
			data["GPU"+std::to_string(i)] = t;
			if(verbose)
				std::cout << "GPU"+std::to_string(i) << " : " << t << "C" << std::endl;
		}
		
		std::string recvmsg="";
		int link[2];
		pid_t pid;
		char buf[512];

		if (pipe(link)==-1)
		{
			std::cerr << "problem with pipe" << std::endl;
			continue;
		}
		
		if ((pid = fork()) == -1)
		{
			std::cerr << "problem with pipe" << std::endl;
			continue;
		}

		if(pid == 0) // that is a child
		{
			dup2 (link[1], STDOUT_FILENO);
			close(link[0]);
			close(link[1]);
			execl("/usr/bin/ipmitool", "ipmitool", "sensor", (char *)0);
			_exit(-1); // somethig has gone wrong if we are here!
		}
		else
		{
			close(link[1]);
			size_t readsz(0);
			while(readsz = read(link[0], buf, sizeof(buf)))
			{
				for(size_t i=0; i!=readsz; i++)
					recvmsg+=buf[i];
			}
			int status;
			waitpid(pid, &status, 0);
			if(status != 0 | recvmsg.size() == 0)
				std::cerr << "something gone wrong with \"ipmitool sensor\"" << std::endl;
			if(verbose)
				std::cout << recvmsg; 
		}
		//parse ipmitool message
		std::stringstream ss(recvmsg);
		std::string l;
		while(getline (ss, l))
		{
				std::stringstream lss(l);
				std::string i;
				int count(0);
				std::string name;
				double value(0.0);
				bool valid(false);
				while(lss >> i)
				{
					count++;
					if(count == 3)
					{
						value=atof(i.c_str());
						break;
					}
					if(count == 1)
						name=i;
					if(count == 2 & i == "|")
						valid = true;
				}
				if(valid & value != 0.0)
				{
					data[name] = value;
					if(verbose)
						std::cout << name << " : " << value << std::endl;
				}
		}
		*data_ready = 1;

		if(verbose)
		{
			for(const auto& it : data)
				std::cout << it.first << " : " << it.second << std::endl;
		}
		
		sleep(13);
		*data_ready = 0;
		pthread_mutex_lock(&trigger_mutex);
	}
	
	s->stop();
	
	delete[] quit;
	
  if(nvmlShutdown() != NVML_SUCCESS)
  {
    std::cerr << "failed to switch nvm off" << std::endl;
    return -1;
  }
	return 0;
}
