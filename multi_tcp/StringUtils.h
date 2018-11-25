#ifndef __StringUtils_H__
#define __StringUtils_H__

#include <string>
#include <cstdio>
#include <stdarg.h>
#include <SDL.h>

namespace StringUtils
{
	std::string format(const char*format,...);
}

#endif
