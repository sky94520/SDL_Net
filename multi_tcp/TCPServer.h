#ifndef __TCPServer_H__
#define __TCPServer_H__
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>

#include "SDL.h"
#include "SDL_net.h"

#include "tcputil.h"
#include "StringUtils.h"

struct Client
{
	std::string name;
	TCPsocket socket;
public:
	Client(const std::string& name, TCPsocket socket)
		:name(name),
		 socket(socket)
	{}
};

/*TCP服务器端，可有多个客户端*/
class TCPServer
{
private:
	//服务器和多个客户端
	TCPsocket _server;
	std::vector<Client> _clients;
	SDLNet_SocketSet _set;
	unsigned int _setNum;
public:
	TCPServer();
	~TCPServer();

	bool init(Uint16 port);
	/**
	 * 监听
	 * @param dt 一帧的时间
	 * @param timeout 检测套接字集合的毫秒
	 */
	void update(float dt, Uint32 timeout);
	std::vector<Client>::iterator doCommand(const std::string& msg, Client* client);
	/**
	 * 发送信息给所有的客户端 如果发送失败则移除该client
	 * @param text 发送的信息
	 */
	void sendAll(const std::string& text);
	/**
	 * 给对应的名字的client发送信息
	 * @param name 对应名字的客户端
	 * @param text 要发送的文本
	 * @return 发送成功返回true，否则返回false
	 */
	bool sendTo(const std::string& name, const std::string& text);
private:
	//创建或者扩展socketSet
	void checkSocketSet();
	//如果名称合法，则添加该客户端
	Client* addClient(TCPsocket client, const std::string& name);
	//移除客户端
	std::vector<Client>::iterator removeClient(std::vector<Client>::iterator);
	std::vector<Client>::iterator removeClient(Client* client);
	//用户名是否唯一
	bool isUniqueNick(const std::string& name);
};
#endif
