#include <cstdio>
#include <string>
#include <SDL.h>
#include <SDL_net.h>

#include <termios.h>
#include <unistd.h>
#include <sys/types.h>

#include "tcputil.h"

using namespace std;

/*linux下需要自行配置，Windows下可#include <conio.h>*/
int kbhit (void)
{
	struct timeval tv;
	fd_set rdfs;
	
	//无等待
	memset(&tv, 0, sizeof(tv));

	FD_ZERO(&rdfs);
	FD_SET(fileno(stdin), &rdfs);

	select(fileno(stdin) + 1, &rdfs, NULL, NULL, &tv);
	return FD_ISSET(fileno(stdin), &rdfs);
}

int main(int argc, char**argv)
{
	IPaddress ip;
	TCPsocket socket;
	SDLNet_SocketSet set;
	bool running = true;
	char text[1024];
	
	const char* host = "localhost";
	Uint16 port = 2000;
	const char* name = "sky";

	if (argc > 1)
		host = argv[1];
	if (argc > 2)
		port = (Uint16)atoi(argv[2]);
	if (argc > 3)
		name = argv[3];

	SDL_Init(0);
	SDLNet_Init();
	
	if (SDLNet_ResolveHost(&ip, host, port) != 0)
	{
		printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		return 1;
	}
	socket = SDLNet_TCP_Open(&ip);
	set = SDLNet_AllocSocketSet(1);
	if (socket == nullptr || set == nullptr)
	{
		printf("error: %s\n", SDLNet_GetError());
		return 1;
	}
	//返回设置成功的个数 -1为错误
	if (SDLNet_TCP_AddSocket(set, socket) == -1)
	{
		printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
		return 1;
	}
	//先发送名称
	if (putMsg(socket, name) == 0)
	{
		SDLNet_TCP_Close(socket);
		return 1;
	}

	while (running)
	{
		int numReady = SDLNet_CheckSockets(set, 100);
		char* str = nullptr;

		if (numReady == -1)
		{
			printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
			break;
		}
		if (numReady == 1 && SDLNet_SocketReady(socket))
		{
			if (getMsg(socket, &str) == nullptr)
				break;
			printf("%s\n", str);
		}
		//用户输入
		if (kbhit() != 0)
		{
			if (!fgets(text, 1024, stdin))
				break;
			//循环删去换行符等
			while (strlen(text) && strchr("\n\r\t", text[strlen(text) - 1]))
				text[strlen(text) - 1] = '\0';
			if (strlen(text))
				putMsg(socket, text);
		}
	}
	SDLNet_TCP_Close(socket);
	SDLNet_FreeSocketSet(set);
	SDLNet_Quit();
	SDL_Quit();
	return 0;
}
