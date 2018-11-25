#include "StringUtils.h"

namespace StringUtils
{
std::string format(const char*format,...)
{
        std::string ret;
        va_list args;
        va_start(args,format);
        char*buf = (char*)malloc(1024*100);
        if(buf)
        {
                SDL_vsnprintf(buf,1024*100,format,args);
                ret = buf;
                free(buf);
        }
        va_end(args);
        return ret;
}
}
