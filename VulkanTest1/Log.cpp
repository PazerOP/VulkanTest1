#include "Log.h"

#include "FixedWindows.h"
#include "stdext.h"

void Log::Msg(const char* str)
{
	OutputDebugStringA(str);
}

void Log::Msg(const wchar_t* str)
{
	OutputDebugStringW(str);
}
