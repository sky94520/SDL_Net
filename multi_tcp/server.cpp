#include<iostream>
#include "TCPServer.h"

int main(int argc, char** argv)
{
	TCPServer* server = new TCPServer();
	bool running = true;

	server->init(2000);
	
	while (running)
	{
		server->update(0.016f, 1000);
	}

	delete server;
}
