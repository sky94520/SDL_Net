#include "TCPServer.h"

TCPServer::TCPServer()
	:_server(nullptr),
	 _set(nullptr),
	 _setNum(0)
{
}

TCPServer::~TCPServer()
{
	if (_set != nullptr)
	{
		SDLNet_FreeSocketSet(_set);
		_set = nullptr;
	}
	for (auto it = _clients.begin(); it != _clients.end();)
	{
		SDLNet_TCP_Close(it->socket);
		it = _clients.erase(it);
	}
	if (_server != nullptr)
	{
		SDLNet_TCP_Close(_server);
		_server = nullptr;
	}
}

bool TCPServer::init(Uint16 port)
{
	IPaddress ip;

	if (SDLNet_Init() != 0)
	{
		printf("SDLNet_Init:%s\n", SDLNet_GetError());
		return false;
	}
	//填充IPaddress
	if (SDLNet_ResolveHost(&ip, nullptr, port) != 0)
	{
		printf("SDLNet_ResolveHost:%s\n", SDLNet_GetError());
		return false;
	}
	//output
	Uint32 ipaddr = SDL_SwapBE32(ip.host);
	printf("IP Address: %d.%d.%d.%d\n",
			ipaddr>>24,
			(ipaddr>>16) & 0xff,
			(ipaddr>>8) & 0xff,
			(ipaddr & 0xff));
	//获取域名
	const char* host = SDLNet_ResolveIP(&ip);

	if (host != nullptr)
		printf("Hostname : %s\n", host);
	else
		printf("Hostname : N/A\n");
	//创建服务器套接字
	_server = SDLNet_TCP_Open(&ip);

	return true;
}

void TCPServer::update(float dt, Uint32 timeout)
{
	int numReady = 0;
	TCPsocket socket = nullptr;

	this->checkSocketSet();
	//检测套接字集合中积极的套接字个数
	numReady = SDLNet_CheckSockets(_set, timeout);

	if (numReady == -1)
	{
		printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
		return ;
	}
	//没有积极的套接字 退出
	if (numReady == 0)
		return ;

	//服务器积极 代表有客户端连接
	if (SDLNet_SocketReady(_server))
	{
		numReady--;
		//尝试获取client
		if ((socket = SDLNet_TCP_Accept(_server)) != nullptr)
		{
			char* name = nullptr;

			//从客户端获取名称
			if (getMsg(socket, &name) != nullptr)
			{
				Client* client = this->addClient(socket, name);

				if (client != nullptr)
					doCommand("WHO", client);
			}
			else
			{
				SDLNet_TCP_Close(socket);
			}
		}
	}
	//遍历客户端 即获取信息
	char* message = nullptr;
	for (auto it = _clients.begin(); numReady != 0 && it != _clients.end();)
	{
		std::string name = it->name;
		TCPsocket socket = it->socket;
		auto it2 = _clients.end();

		if (SDLNet_SocketReady(socket))
		{
			//获取文本
			if (getMsg(socket, &message) != nullptr)
			{
				numReady--;
				auto index = it - _clients.begin();
				//命令 执行某些命令可能会使得迭代器失效
				if (message[0] == '/' && strlen(message) > 1)
				{
					it2 = doCommand(message + 1, &_clients[index]);
				}
				else
				{
					auto text = StringUtils::format("<%s>%s%",
							name.c_str(),
							message);
					printf("<%s> says:%s\n", name.c_str(), message);
					sendAll(text);
				}
			}
			else
			{
				it = this->removeClient(it);
			}
		}
		it = (it2 == _clients.end()) ? ++it : it2;
	}
}

std::vector<Client>::iterator TCPServer::doCommand(const std::string& msg, Client* client)
{
	if (msg.empty() || client == nullptr)
		return _clients.end();
	//找到第一个空格
	auto first = msg.find(' ');
	std::string command;

	//获取命令
	if (first != std::string::npos)
		command = msg.substr(0, first).c_str();
	else
		command = msg.c_str();
	if (strcasecmp(command.c_str(), "NICK") == 0)
	{
		if (first == std::string::npos)
		{
			std::string text = "Invalid Nickname!";
			putMsg(client->socket, text.c_str());
		}
		else
		{
			auto oldName = client->name;
			auto name = msg.substr(first + 1);
			std::string text;

			if (!this->isUniqueNick(name))
			{
				text = "Duplicate Nickname!";
				putMsg(client->socket, text.c_str());
			}
			else
			{
				client->name = name;
				text = StringUtils::format("%s->%s", oldName.c_str(), name.c_str());
				sendAll(text);
			}
		}
	}//退出
	else if (strcasecmp(command.c_str(), "QUIT") == 0)
	{
		if (first != std::string::npos)
		{
			auto text = msg.substr(first + 1);
			text = StringUtils::format("%s quits : %s", client->name.c_str(), text.c_str());
			sendAll(text);
		}
		else
		{
			auto text = StringUtils::format("%s quits", client->name.c_str());
			sendAll(text);
		}
		return this->removeClient(client);
	}//client =》client
	else if (strcasecmp(command.c_str(), "MSG") == 0)
	{
		if (first == std::string::npos)
		{
			putMsg(client->socket, "Format:/MSG Nickname message");
		}
		else
		{
			auto second = msg.find(' ', first + 1);
			std::string name = msg.substr(first + 1, second - first - 1);
			auto text = msg.substr(second + 1);
			text = StringUtils::format("<%s> %s", name.c_str(), text.c_str());
			//发送到
			if (!this->sendTo(name, text))
				putMsg(client->socket, "no found the client of name");
		}
	}//输出谁在线
	else if (strcasecmp(command.c_str(), "WHO") == 0)
	{
		IPaddress* ipaddr = nullptr;
		Uint32 ip;
		std::string text;

		for (auto it = _clients.begin(); it != _clients.end(); it++)
		{
			//除去自己
			if (it->name == client->name)
				continue;
			ipaddr = SDLNet_TCP_GetPeerAddress(it->socket);
			if (ipaddr == nullptr)
				continue;
			ip = SDL_SwapBE32(ipaddr->host);

			text = StringUtils::format("%s %u.%u.%u.%u:%u", it->name.c_str(),
					ip>>24,
					(ip>>16) & 0xff,
					(ip>>8) & 0xff,
					ip & 0xff,
					ipaddr->port);

			putMsg(client->socket, text.c_str());
		}
	}
	else
	{
		auto text = StringUtils::format("Invalid Command:%s", command.c_str());
		putMsg(client->socket, text.c_str());
	}
	return _clients.end();
}

void TCPServer::sendAll(const std::string& text)
{
	if (text.empty() || _clients.size() == 0)
		return ;
	for (auto it = _clients.begin(); it != _clients.end();)
	{
		auto& client = *it;
		TCPsocket socket = client.socket;

		putMsg(socket, text.c_str());
		it++;
	}
}

bool TCPServer::sendTo(const std::string& name, const std::string& text)
{
	//查找
	auto it = find_if(_clients.begin(), _clients.end(), [&name](const Client& client)
	{
		return name == client.name;
	});

	if (it == _clients.end())
		return false;
	putMsg(it->socket, text.c_str());

	return true;
}

void TCPServer::checkSocketSet()
{
	bool ret = false;

	if (_set == nullptr)
	{
		_set = SDLNet_AllocSocketSet(_clients.size() + 1);
		ret = true;
	}
	else if (_setNum != _clients.size() + 1)
	{
		SDLNet_FreeSocketSet(_set);
		_set = SDLNet_AllocSocketSet(_clients.size() + 1);
		ret = true;
	}
	//只有在重新创建时才会填充
	if (!ret)
		return;
	_setNum = _clients.size() + 1;
	SDLNet_TCP_AddSocket(_set, _server);

	for (auto it = _clients.begin(); it != _clients.end(); it++)
		SDLNet_TCP_AddSocket(_set, it->socket);
}

Client* TCPServer::addClient(TCPsocket socket, const std::string& name)
{
	//名称为空
	if (name.empty())
	{
		char text[] = "Invalid Nickname...bye bye!";
		putMsg(socket, text);
		SDLNet_TCP_Close(socket);
		return nullptr;
	}

	if (!this->isUniqueNick(name))
	{
		char text[] = "Duplicate Nickname...bye bye!";
		putMsg(socket, text);
		SDLNet_TCP_Close(socket);
		return nullptr;
	}
	//添加
	_clients.push_back(Client(name, socket));
	printf("--> %s\n", name.c_str());

	sendAll(StringUtils::format("--->%s", name.c_str()));

	return &_clients.back();
}

std::vector<Client>::iterator TCPServer::removeClient(std::vector<Client>::iterator it)
{
	const std::string& name = it->name;
	TCPsocket socket = it->socket;

	it = _clients.erase(it);
	SDLNet_TCP_Close(socket);
	//发送数据
	printf("<-- %s\n", name.c_str());
	std::string text = StringUtils::format("<--%s", name.c_str());
	sendAll(text.c_str());

	return it;
}

std::vector<Client>::iterator TCPServer::removeClient(Client* client)
{
	auto it = find_if(_clients.begin(), _clients.end(), [client](const Client& c)
	{
		return c.name == client->name;
	});

	return this->removeClient(it);
}

bool TCPServer::isUniqueNick(const std::string& name)
{
	auto it = find_if(_clients.begin(), _clients.end(), [&name](const Client& client)
	{
		return client.name == name;
	});
	return it == _clients.end();
}
