#pragma once

#include <string>
#include "StringTools.h"

enum class LogType
{
	Misc = (1 << 0),
	ObjectLifetime = (1 << 1),
};

class Log final
{
public:
	Log() = delete;
	Log(const Log&) = delete;
	Log(Log&&) = delete;
	~Log() = delete;

	template<LogType type = LogType::Misc, class... Args> static void TagMsg(std::string tag, const std::string& fmt, Args... args)
	{
		MsgRaw(type, tag.append(StringTools::CSFormat(fmt, args...)).append("\n"sv));
	}

	template<LogType type = LogType::Misc, class... Args> static void Msg(const char* fmt, const Args&... args)
	{
		MsgRaw(type, StringTools::CSFormat(fmt, args...).append("\n"sv));
	}
	template<LogType type = LogType::Misc, class... Args> static void Msg(const std::string_view& fmt, const Args&... args)
	{
		MsgRaw(type, StringTools::CSFormat(std::string(fmt).append("\n"sv), args...));
	}
	template<LogType type = LogType::Misc, class... Args> static void Msg(std::string fmt, const Args&... args)
	{
		MsgRaw(type, StringTools::CSFormat(fmt.append("\n"sv, args...)));
	}

	template<LogType type = LogType::Misc, size_t charsPerLine = 80, class... Args> static void BlockMsg(const char* fmt, const Args&... args)
	{
		BlockMsgRaw(type, StringTools::CSFormat(fmt, args...), charsPerLine);
	}
	template<LogType type = LogType::Misc, size_t charsPerLine = 80, class... Args> static void BlockMsg(const std::string& fmt, const Args&... args)
	{
		BlockMsgRaw(type, StringTools::CSFormat(fmt, args...), charsPerLine);
	}

	static void EnableType(LogType type);
	static bool IsTypeEnabled(LogType type);
	static void DisableType(LogType type);

private:
	static void MsgRaw(LogType type, const std::string& str);
	static void BlockMsgRaw(LogType type, std::string str, size_t charsPerLine = 80);

	static LogType s_Types;
};