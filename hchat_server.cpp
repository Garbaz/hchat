#include <iostream>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define PORT "1729"
#define MAX_CLIENTS 50
#define BUFFER_LEN 256
#define QUIT(ret) return ret;
#define debug if(DO_DEBUG)cout

const bool DO_DEBUG = 1;

using namespace std;

void* handle_client(void* id);
void broadcast_msg(string msg, int id);
int get_free_id();
void disconnect(int id, char* &buffer);
string toStr(int in);

int status, sockfd, clientfd[MAX_CLIENTS], clients, clientids[MAX_CLIENTS], currentid;
bool is_reader[MAX_CLIENTS];
string nicks[MAX_CLIENTS];
struct addrinfo hints, *servinfo;
struct sockaddr_storage conn_addr[MAX_CLIENTS];
socklen_t conn_addr_size[MAX_CLIENTS];
pthread_t client_threads[MAX_CLIENTS];

int main(int argc, char* argv[])
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		is_reader[i] = 0;
		clientfd[i] = -1;
		clientids[i] = -1;
	}
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	
	if((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
	{
		cerr << "{2} getaddrinfo error: " << gai_strerror(status) << endl;
		QUIT(2);
	}
	
	if((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
	{
		cerr << "{3} socket error: Could not get socket file descriptor" << endl;
		QUIT(3);
	}
	
	int yes = 1;
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
		perror("setsockopt");
		QUIT(8);
	}
	
	if(bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
	{
		cerr << "{4} bind error: Unable to bind to port; Try again in a minute" << endl;
		QUIT(4);
	}
	
	while(1)
	{
		if(clients < MAX_CLIENTS)
		{
			cout << "Waiting for client connection..." << endl;
			
			if((currentid = get_free_id()) == -1)
			{
				cerr << "{8} clientid error: Can't get a client id" << endl;
				continue;
			}
			
			if(listen(sockfd, MAX_CLIENTS) == -1)
			{
				cerr << "{5} listen error: Unable to listen" << endl;
				continue;
//				QUIT(5);
			}
			
			conn_addr_size[currentid] = sizeof conn_addr[currentid];
			if((clientfd[currentid] = accept(sockfd, (struct sockaddr*)&conn_addr[currentid], &conn_addr_size[currentid])) == -1)
			{
				cerr << "{6} accept error: Unable to accept incoming connection" << endl;
				continue;
//				QUIT(6);
			}
			
			clientids[currentid] = currentid;
			
			if(pthread_create(&client_threads[currentid], NULL, handle_client, (void*)&clientids[currentid]) == -1)
			{
				cerr << "{7} thread error: Unable to create new client thread" << endl;
				clientids[currentid] = -1;
				continue;
//				QUIT(7);
			}
			
			cout << "Client (ID=" << currentid << ", FD=" << clientfd[currentid] <<") successfully connected!" << endl;
			
			clients++;
		}
	}
	return 0;
}

void* handle_client(void* id)
{
	int myid = *((int*)id);
	int bytes_recieved = -1;
	char* buffer = (char*) malloc(BUFFER_LEN);
	
	bytes_recieved = recv(clientfd[myid], buffer, BUFFER_LEN, 0);
	cout << "Recieved " << bytes_recieved << " bytes from client ["<< myid << ']' << endl;
	if(buffer[0] == 'R' && buffer[1] == 'D' && buffer[2] == 'R')
	{
		is_reader[myid] = 1;
		while(1)
		{
			if((bytes_recieved = recv(clientfd[myid], buffer, BUFFER_LEN, 0)) == -1)
			{
				continue;
			}
			string mmsg = "";
			for(int i = 0; i < bytes_recieved; i++)
			{
				mmsg += buffer[i];
			}
			
			if(mmsg.substr(0,7) == "stp_rdg")
			{
				break;
			}
		}
	}
	else
	{
		is_reader[myid] = 0;
		
		nicks[myid] = string(buffer).substr(3, bytes_recieved - 4);
		
		
		broadcast_msg("----" + nicks[myid] + " joined----\n", -1);
		do
		{
			if((bytes_recieved = recv(clientfd[myid], buffer, BUFFER_LEN, 0)) == -1)
			{
				continue;
			}
			string mmsg = "";
			for(int i = 0; i < bytes_recieved; i++)
			{
				mmsg += buffer[i];
			}
			cout << "Recieved " << bytes_recieved << " bytes from " << nicks[myid] << " ["<< myid << ']' << endl;
			broadcast_msg(mmsg, myid);
		}while(bytes_recieved != 0);
	}
	disconnect(myid, buffer);
	pthread_exit(NULL);
}

void broadcast_msg(string msg, int id)
{
	string pmsg = msg;
	if(id >= 0) pmsg = nicks[id] + "[" + toStr(id) + "]>> " + msg;
	cout << pmsg;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(is_reader[i] && clientids[i] != -1)
		{
			int bytes_sent;
			do
			{
				if((bytes_sent = send(clientfd[i], pmsg.c_str(), pmsg.length(), 0)) == -1)
				{
					cerr << "{6} send error: Unable to send message to client [" << i << ']' << endl;
				}
			}while(bytes_sent < (int)pmsg.length());
		}
	}
}

void disconnect(int id, char* &buffer)
{
	free(buffer);
	shutdown(clientfd[id],2);
	clientids[id] = -1;
	is_reader[id] = 0;
	clients--;
	cout << nicks[id] << " [" << id << "] disconnected!" << endl;
	nicks[id] = "";
}

int get_free_id()
{
	if(clients == MAX_CLIENTS) return -1;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(clientids[i] == -1) return i;
	}
	return -1;
}

string toStr(int in)
{
	stringstream ss;
	ss << in;
	return ss.str();
}
