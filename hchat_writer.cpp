#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define QUIT(ret) freeaddrinfo(servinfo); return ret;
#define BUFFER_LEN 256
#define PORT "1729"

using namespace std;

int status, sockfd, bytes_sent;
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
	
	while(nick.length() < 7)
	{
		cout << "Choose a Nickname (at least 3 characters): ";
		getline(cin, nick);
		nick = "NCK" + nick + '\n';
	}
	
	do
	{
		if((bytes_sent = send(sockfd, nick.c_str(), nick.length(), 0)) == -1)
		{
			cerr << "{6} send error: Unable to send nickname" << endl;
			continue;
		}
		break;
	}while(1);
	
	
	cout << "Successfully connected to server! Type away:" << endl;
	while(1)
	{
		cout << ">";
		getline(cin, msg);
		
		msg += '\n';
		do
		{
			if((bytes_sent = send(sockfd, msg.c_str(), msg.length(), 0)) == -1)
			{
				cerr << "{6} send error: Unable to send message" << endl;
				QUIT(6);
			}
		}while(bytes_sent < (int)msg.length());
	}
	
	freeaddrinfo(servinfo);
	
	return 0;
}
