#define defPORT 8080
#include <string>

typedef int soc_handler_type;

struct soc
{
	static const char* defIP;
	soc_handler_type sh;
	
	soc() : sh(-1) {}
	
	bool client_start(const char* net_addr=defIP, const int port=defPORT);
// you can just bind the socket and do listen_new_client for any new connected client socket.
// this alows to make multi client server using pthreads
	bool srv_bind(const char* net_addr=defIP, const int port=defPORT);
	
	soc listen_new_client(); //returns client's handler
	
	bool srv_start(const char* net_addr=defIP, const int port=defPORT); //returns client's handler

	void Close();

	static void Send(soc cl, const std::string&);

	static void Read(soc cl, std::string&);

/*
//functions to work with big amount of data:
	void send_big(int cl, double* obj, int N);
	void read_big(int cl,double* obj,int N);

	void read_simple(int cl,double* obj,int N);

	void send_simple(int cl,double* obj,int N);


	void recv(int cl,int* txt);
*/
};

bool operator<(const soc& a, const soc& b); // for map
