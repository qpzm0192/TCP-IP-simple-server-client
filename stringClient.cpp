#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include <iostream>
#include <limits.h>
#include <sstream>
#include <iomanip>

using namespace std;

#define MAX_BUF USHRT_MAX


int main(int argc, char *argv[])
{
    int sockfd, bytes_sent, bytes_recv;  
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
	char *port;
	char *address;
	
	port = getenv("SERVER_PORT");
	address = getenv("SERVER_ADDRESS");

	////////////////////////////////////////////////////////////////////////////////////
	// Code of making connection is reused from <Beej's Guide to Network Programming> //
	////////////////////////////////////////////////////////////////////////////////////	
	
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(address, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) { //socket
            perror("client: socket");
            continue;
        }
	
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) { //connect
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    freeaddrinfo(servinfo); 
	
	string msg;
	while(getline(cin, msg)) {
		cout << msg << endl;
		
		pid_t pid = fork();
		
		if (pid == 0) {
			sleep(2);
			
			char buffer[MAX_BUF];
			unsigned long len;
			string s;
			
			//convert length to 4-byte
			len = msg.length() + 1;
			unsigned char bytes[4];
			bytes[0] = (len >> 24) & 0xFF;
			bytes[1] = (len >> 16) & 0xFF;
			bytes[2] = (len >> 8) & 0xFF;
			bytes[3] = len & 0xFF;
			string hex_len(reinterpret_cast<char*>(bytes), 4);
			
			//append to input
			s.append(hex_len);
			s.append(msg);
			
			s[s.length()] = '\0';
			
			if ((bytes_sent = send(sockfd, s.c_str(), s.length(), 0)) == -1) { //send
				perror("send");
				exit(1);
			}
			
			read(sockfd, bytes, 4);	//read 4-byte length
			
			if ((bytes_recv = recv(sockfd, buffer, MAX_BUF-1, 0)) == -1) { //receive
				perror("recv");
				exit(1);
			}
			
			buffer[bytes_recv] = '\0'; 
			string out(buffer);		
			cout << "Server: " << out << endl;
	
			break;			
		}
	}
	
    close(sockfd);

    return 0;
}