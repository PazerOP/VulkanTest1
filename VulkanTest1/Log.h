#pragma once

#include <string>
#include "StringTools.h"

class Log final
{
public:
	Log() = delete;
	Log(const Log&) = delete;
	Log(Log&&) = delete;
	~Log() = delete;

	template<class... Args> static void Msg(const char* fmt, Args... args)
	{
		MsgRaw(StringTools::CSFormat(fmt, args...).append("\n"sv));
	}
	template<class... Args> static void Msg(const std::string_view& fmt, Args... args)
	{
		MsgRaw(StringTools::CSFormat(std::string(fmt).append("\n"sv), args...));
	}
	template<class... Args> static void Msg(std::string fmt, Args... args)
	{
		MsgRaw(StringTools::CSFormat(fmt.append("\n"sv, args...)));
	}

	template<size_t charsPerLine = 80, class... Args> static void BlockMsg(const char* fmt, Args... args)
	{
		BlockMsgRaw(StringTools::CSFormat(fmt, args...), charsPerLine);
	}
	template<size_t charsPerLine = 80, class... Args> static void BlockMsg(const std::string& fmt, Args... args)
	{
		BlockMsgRaw(StringTools::CSFormat(fmt, args...), charsPerLine);
	}

private:
	static void MsgRaw(const std::string& str);
	static void BlockMsgRaw(std::string str, size_t charsPerLine = 80);
};