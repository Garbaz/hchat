#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define QUIT(ret) freeaddrinfo(servinfo); return ret;
#define BUFFER_LEN 256
#define PORT "1729"

using namespace std;

void interruptHandler(int s);

int status, sockfd, bytes_sent, bytes_recieved;
char* buffer;
string msg, nick;
struct addrinfo hints, *servinfo;

int main(int argc, char* argv[])
{
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	if(argc != 2)
	{
		cerr << "{1} Usage: " << argv[0] << " hostname" << endl;
		QUIT(1);
	}
	
	if((status = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0)
	{
		cerr << "{2} getaddrinfo error: " << gai_strerror(status) << endl;
		QUIT(2);
	}
	
	if((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
	{
		cerr << "{3} socket error: Could not get socket file descriptor" << endl;
		QUIT(3);
	}
	
	if(connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
	{
		cerr << "{5} connect error: Unable to connect to target host" << endl;
		QUIT(5);
	}
	
	struct sigaction sigIntHandler;

   sigIntHandler.sa_handler = interruptHandler;
   sigemptyset(&sigIntHandler.sa_mask);
   sigIntHandler.sa_flags = 0;

   sigaction(SIGINT, &sigIntHandler, NULL);
	
	nick = "RDR\n";
	
	do
	{
		if((bytes_sent = send(sockfd, nick.c_str(), nick.length(), 0)) == -1)
		{
			cerr << "{6} send error: Unable to send RDR" << endl;
			continue;
		}
		break;
	}while(1);
	
	buffer = (char*) malloc(BUFFER_LEN);
	
	cout << "Successfully connected to server! Read away:" << endl;
	
	while(1)
	{
		if((bytes_recieved = recv(sockfd, buffer, BUFFER_LEN, 0)) == -1)
			{
				continue;
			}
			
			for(int i = 0; i < bytes_recieved; i++)
			{
				putchar(buffer[i]);
			}
	}
	
	freeaddrinfo(servinfo);
	
	return 0;
}

void interruptHandler(int s)
{
	string quit_msg = "stp_rdg\n";
	free(buffer);
	send(sockfd, quit_msg.c_str(), quit_msg.length(), 0);
	freeaddrinfo(servinfo);
	exit(1);
}
