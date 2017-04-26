#pragma once
#include <string>

extern std::string vstrprintf(const char* fmt, va_list args);

inline std::string to_string(const char* str) { return std::string(str); }
inline std::wstring to_wstring(const wchar_t* str) { return std::wstring(str); }

template<class... Args> std::string StringFormat(std::string fmt, Args... args)
{
	using namespace std;

	std::string args[] = { to_string(args)... };

	constexpr size_t argCount = sizeof(args) / sizeof(args[0]);

	for (size_t i = 0; i < argCount; i++)
	{
		char buf[32];
		const auto idLength = sprintf_s(buf, "{%zi}", i);

		for (size_t c = 0; c < fmt.size(); c++)
		{
			if (!fmt.compare(c, idLength, buf))
			{

			}
		}
	}

	return fmt;
}
template<class... Args> std::wstring StringFormat(std::wstring fmt, Args... args)
{
	using namespace std;

	std::wstring args[] = { to_wstring(args)... };

	constexpr size_t argCount = sizeof(args) / sizeof(args[0]);

	for (size_t i = 0; i < argCount; i++)
	{
		wchar_t buf[32];
		swprintf_s(buf, L"\\{%zi}", i);

		std::regex r(buf);
		std::regex_replace(fmt, r, args[i].c_str());
	}
}

extern std::string strprintf(const char* fmt, ...);