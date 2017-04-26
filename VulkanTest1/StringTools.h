#pragma once
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_literals;

class StringTools final
{
public:
	StringTools() = delete;
	StringTools(const StringTools& other) = delete;
	StringTools(StringTools&& other) = delete;
	~StringTools() = delete;

	static void UnitTests();

	template<class Traits = std::char_traits<char>, class Alloc = std::allocator<char>, class... Args> static std::basic_string<char, Traits, Alloc> CSFormat(const char* fmt, Args... args);
	template<class Traits, class Alloc, class... Args> static std::basic_string<char, Traits, Alloc> CSFormat(std::basic_string<char, Traits, Alloc> fmt, Args... args);

	static bool IsEscaped(const char* str, size_t offset, char escapeChar = U'\\');
	static bool IsEscaped(const std::string_view& str, size_t offset, char32_t escapeChar = U'\\');
	static std::vector<bool> GetEscapeData(const char* str)

private:

	struct CSToken
	{
		size_t m_ID;
		std::string_view m_Data;
		std::string_view m_Mode;
	};

	static std::vector<CSToken> ParseCSTokens(const std::string_view& str);
};

template<class Traits, class Alloc, class... Args>
inline std::basic_string<char, Traits, Alloc> StringTools::CSFormat(const char* fmt, Args... args)
{
	return CSFormat(std::basic_string<char, Traits, Alloc>(fmt), args...);
}

template<class Traits, class Alloc, class ...Args>
inline std::basic_string<char, Traits, Alloc> StringTools::CSFormat(std::basic_string<char, Traits, Alloc> fmt, Args... args)
{
	return std::basic_string<char, Traits, Alloc>();
}