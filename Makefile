all: server client

server: stringServer

client: stringClient

stringServer: stringServer.cpp
	g++ stringServer.cpp -o stringServer 

stringClient: stringClient.cpp
	g++ stringClient.cpp -o stringClient
	
