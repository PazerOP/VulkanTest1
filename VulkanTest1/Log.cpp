#include "Log.h"

#include "FixedWindows.h"
#include "stdext.h"

void Log::Msg(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	std::string result = vstrprintf(fmt, args) + '\n';

	OutputDebugStringA(result.c_str());
}
