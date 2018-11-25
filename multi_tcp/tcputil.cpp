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

std::string getLocalHostIP(Uint32& num_ip)
{
#ifdef WIN32
	char text[20] = {};
	IPaddress localhost_ip;
	SDLNet_ResolveHost(&localhost_ip, nullptr, 0);
	SDLNet_ResolveHost(&localhost_ip, SDLNet_ResolveIP(&localhost_ip), 0);
	//output
	num_ip = SDL_SwapBE32(localhost_ip.host);
	sprintf(text, "%d.%d.%d.%d",
			num_ip>>24,
			(num_ip>>16) & 0xff,
			(num_ip>>8) & 0xff,
			(num_ip & 0xff));
	return std::string(text);
#else
	std::string ip;
	int socket_fd = socket(PF_INET,SOCK_DGRAM,0);
        struct sockaddr_in *sin;
        ifconf conf;
        struct ifreq* ifr;
        char buf[128];
        int i,n;
        conf.ifc_len = 128;
        conf.ifc_buf = buf;
        ioctl(socket_fd,SIOCGIFCONF,&conf);
        ifr = conf.ifc_req;
        n = conf.ifc_len/sizeof(struct ifreq);
        for(i=0;i<n;i++)
        {
                sin = (struct sockaddr_in*)(&ifr->ifr_addr);
                ip = inet_ntoa(sin->sin_addr);
                if(ip.compare("127.0.0.1"))
                {
                        return ip;
                }
                ifr++;
        }
        ip = "127.0.0.1";
	return ip;
#endif
}
