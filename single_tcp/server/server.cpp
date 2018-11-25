#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SDL_net.h"

#define SIZE 1024
int main(int argc, char** argv)
{
	IPaddress ip;
	TCPsocket server_socket, client_socket;
	bool quit = false;
	char buffer[SIZE];
	//默认端口
	Uint16 port = 2000;

	SDL_Init(0);

	if (argc < 2)
	{
		printf("the server wiil use the default port:%u\n", port);
	}
	else
		port = (Uint16)atoi(argv[1]);

	printf("the server will use port:%u\n", port);
	SDLNet_Init();
	//创建一个服务器类型的IPaddress
	if (SDLNet_ResolveHost(&ip, NULL, port) != 0)
	{
		printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		return 1;
	}
	//printf("current server:\nhost:%u\tport:%u\n", ip.host, ip.port);

	//打开一个TCP连接
	if ((server_socket = SDLNet_TCP_Open(&ip)) == nullptr)
	{
		printf("error:%s\n", SDLNet_GetError());
		return 2;
	}
	printf("create server success\n");

	while (!quit)
	{
		//存在TCP连接
		if ((client_socket = SDLNet_TCP_Accept(server_socket)) != nullptr)
		{
			IPaddress* remoteIP = nullptr;

			//获取远程ip
			if ((remoteIP = SDLNet_TCP_GetPeerAddress(client_socket)) != nullptr)
			{
				printf("remote ip is %x, port %u\n"
					, SDLNet_Read32(&remoteIP->host)
					, SDLNet_Read16(&remoteIP->port));
			}
			bool running = true;

			//监听
			while (running)
			{
				printf("waiting data\n");
				if (SDLNet_TCP_Recv(client_socket, buffer, SIZE) > 0)
				{
					printf("Client say: %s\n", buffer);

					if (strcmp(buffer, "exit") == 0)
					{
						running = false;
						printf("Terminal\n");
					}
					else if (strcmp(buffer, "quit") == 0)
					{
						quit = true;
						running = false;
						printf("Quit\n");
					}
				}
			}
			SDLNet_TCP_Close(client_socket);
		}
	}

	printf("server success\n");
	SDLNet_TCP_Close(server_socket);
	SDLNet_Quit();
	return 0;
}
