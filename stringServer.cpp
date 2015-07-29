#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sstream>
#include <limits.h>
#include <string.h>

using namespace std;

#define MAX_LEN 100
#define MAX_BUF USHRT_MAX
#define PORT 0
#define MAX_CLIENT 5


string process(string s) {
	stringstream ss;
	ss << s;
	int len = s.length();
	bool capital = 1;
	for(int i=0; i<len; i++) {
		char space = s[i];
		if(space == 32) {
			capital = 1;
			continue;
		}
		char c;
		ss >> c;
		int ic = (int) c;
		if(capital) {
			if(ic >= 97 && ic <= 122) {
				s[i] = c - 32;
				capital = 0;
			} else {
				capital = 0;
				continue;
			}
		} else {
			if(ic >= 65 && ic <= 90) {
				s[i] = c + 32;				
			} else {
				continue;
			}
		}
	}
	return s;
}

int main() {
	char hostname[MAX_LEN];
	char remoteIP[INET6_ADDRSTRLEN];
	int listenfd, connfd, n, bytes_sent, bytes_recv, i, j, fdmax, newfd, result;
	socklen_t clilen;
	fd_set master, read_fds;
	socklen_t addrlen;
	struct sockaddr_in server_addr;
	struct sockaddr_storage remoteaddr;
	
	////////////////////////////////////////////////////////////////////////////////////
	// Code of making connection is reused from <Beej's Guide to Network Programming> //
	////////////////////////////////////////////////////////////////////////////////////
	
	listenfd = socket(AF_INET, SOCK_STREAM, 0); //socket
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);
	bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr)); //bind

	listen(listenfd, MAX_CLIENT); //listen
	
	//print SERVER_ADDRESS
	gethostname(hostname,MAX_LEN);
	cout << "SERVER_ADDRESS " << hostname  << endl;
	
	//print SERVER_PORT
	addrlen = sizeof(server_addr);
	if (getsockname(listenfd, (struct sockaddr *)&server_addr, &addrlen) == -1) perror("getsockname");	
	cout << "SERVER_PORT " << ntohs(server_addr.sin_port) << endl;	
	
	///////////////////////////////////////////////////////////////////////////
	// Code of select() is reused from <Beej's Guide to Network Programming> //
	///////////////////////////////////////////////////////////////////////////
	
	// add the listener to the master set
    FD_SET(listenfd, &master);

    // keep track of the biggest file descriptor
    fdmax = listenfd; // so far, it's this one
	while(1) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listenfd) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
					
                    newfd = accept(listenfd, (struct sockaddr *)&remoteaddr, &addrlen); //accept
                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (fdmax < newfd) {    
                            fdmax = newfd;
                        }
                    }
                } else {
					char buffer[MAX_BUF];
					char len[4];
					
					read(i, len, 4); //read 4-byte length
					if ((bytes_recv = recv(i, buffer, MAX_BUF, 0)) <= 0) { //receive
						if (bytes_recv != 0) {
							perror("recv");
						} 
						close(i);
						FD_CLR(i, &master);
					} else {
						//process data
						string hex_len(reinterpret_cast<char*>(len), 4);
						string reply(hex_len);
						reply.append(process(string(buffer)));
						
						reply[reply.length()] = '\0';
						
						if((bytes_sent = send(i, reply.c_str(), reply.length(), 0)) == -1) { //send
							perror("sent");
							exit(1);
						}
						memset(buffer, 0, sizeof(buffer));
					}
                } 
            } 
        } 
    } 
}