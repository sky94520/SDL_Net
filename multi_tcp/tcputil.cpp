#include "tcputil.h"

char *getMsg(TCPsocket sock, char **buf)
{
	Uint32 len,result;
	static char *_buf;

	/* allow for a NULL buf, use a static internal one... */
	if(!buf)
		buf=&_buf;
	
	/* free the old buffer */
	if(*buf)
		free(*buf);
	*buf=NULL;

	/* receive the length of the string message */
	result=SDLNet_TCP_Recv(sock,&len,sizeof(len));
	if(result<sizeof(len))
	{
		if(SDLNet_GetError() && strlen(SDLNet_GetError())) /* sometimes blank! */
			printf("SDLNet_TCP_Recv: %s\n", SDLNet_GetError());
		return(NULL);
	}
	
	/* swap byte order to our local order */
	len=SDL_SwapBE32(len);
	
	/* check if anything is strange, like a zero length buffer */
	if(!len)
		return(NULL);

	/* allocate the buffer memory */
	*buf=(char*)malloc(len);
	if(!(*buf))
		return(NULL);

	/* get the string buffer over the socket */
	result=SDLNet_TCP_Recv(sock,*buf,len);
	if(result<len)
	{
		if(SDLNet_GetError() && strlen(SDLNet_GetError())) /* sometimes blank! */
			printf("SDLNet_TCP_Recv: %s\n", SDLNet_GetError());
		free(*buf);
		buf=NULL;
	}

	/* return the new buffer */
	return(*buf);
}

int putMsg(TCPsocket sock,const char *buf)
{
	Uint32 len,result;

	if(!buf || !strlen(buf))
		return(1);

	/* determine the length of the string */
	len=strlen(buf)+1; /* add one for the terminating NULL */
	
	/* change endianness to network order */
	len=SDL_SwapBE32(len);

	/* send the length of the string */
	result=SDLNet_TCP_Send(sock,&len,sizeof(len));
	if(result<sizeof(len)) {
		if(SDLNet_GetError() && strlen(SDLNet_GetError())) /* sometimes blank! */
			printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		return(0);
	}
	
	/* revert to our local byte order */
	len=SDL_SwapBE32(len);
	
	/* send the buffer, with the NULL as well */
	result=SDLNet_TCP_Send(sock,buf,len);
	if(result<len) {
		if(SDLNet_GetError() && strlen(SDLNet_GetError())) /* sometimes blank! */
			printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		return(0);
	}
	
	/* return the length sent */
	return(result);
}
