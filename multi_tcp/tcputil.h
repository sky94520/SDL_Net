#ifndef tcputil_h
#define tcputil_h 1

#ifdef WIN32
#else
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <net/if.h>
	#include <arpa/inet.h>
#endif

#include <string>

#include "SDL.h"
#include "SDL_net.h"

/* receive a buffer from a TCP socket with error checking */
/* this function handles the memory, so it can't use any [] arrays */
/* returns 0 on any errors, or a valid char* on success */
extern char *getMsg(TCPsocket sock, char **buf);

/* send a string buffer over a TCP socket with error checking */
/* returns 0 on any errors, length sent on success */
extern int putMsg(TCPsocket sock, const char *buf);
/**
 * 获取当前的IP地址
 * @param num_ip ip的数字
 * @return 格式化的ip地址
 */
extern std::string getLocalHostIP(Uint32& num_ip);

#endif
