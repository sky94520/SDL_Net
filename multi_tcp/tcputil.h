#ifndef tcputil_h
#define tcputil_h 1

#include "SDL.h"
#include "SDL_net.h"

/* receive a buffer from a TCP socket with error checking */
/* this function handles the memory, so it can't use any [] arrays */
/* returns 0 on any errors, or a valid char* on success */
extern char *getMsg(TCPsocket sock, char **buf);

/* send a string buffer over a TCP socket with error checking */
/* returns 0 on any errors, length sent on success */
extern int putMsg(TCPsocket sock, const char *buf);

#endif
