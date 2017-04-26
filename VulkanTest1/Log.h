#pragma once
#include "stdext.h"

class Log final
{
public:
	Log() = delete;
	Log(const Log&) = delete;
	Log(Log&&) = delete;
	~Log() = delete;

	template<class... Args> static void Msg(const char* fmt, Args... args)
	{
		Msg(StringFormat(std::string(fmt), args...).c_str());
	}
	static void Msg(const char* str);

	template<class... Args> static void Msg(const wchar_t* fmt, Args... args)
	{
		Msg(StringFormat(std::wstring(fmt), args...).c_str());
	}
	static void Msg(const wchar_t* str);

private:
};