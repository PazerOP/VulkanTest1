#pragma once

class Log final
{
public:
	Log() = delete;
	Log(const Log&) = delete;
	Log(Log&&) = delete;
	~Log() = delete;

	static void Msg(const char* fmt, ...);
};