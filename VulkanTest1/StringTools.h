#pragma once
#include <string>
#include <string_view>

using namespace std::string_literals;

class StringTools final
{
public:
	StringTools() = delete;
	StringTools(const StringTools& other) = delete;
	StringTools(StringTools&& other) = delete;
	~StringTools() = delete;

	static void UnitTests();

	template<class Elem, class... Args> static std::basic_string<Elem> CSFormat(std::basic_string<Elem> fmt, Args... args);

	static bool IsEscaped(const char* str, size_t offset, char32_t escapeChar = U'\\');
	template<class Elem, class Traits, class Allocator> static bool IsEscaped(const std::basic_string<Elem, Traits, Allocator>& str, size_t offset, Elem escapeChar = '\\');
	template<class Elem, class Traits> static bool IsEscaped(const std::basic_string_view<Elem, Traits>& str, size_t offset, Elem escapeChar = '\\');

private:

	template<class Elem> struct CSToken
	{
		size_t m_ID;
		std::basic_string_view<Elem> m_Data;
		std::basic_string_view<Elem> m_Mode;
	};

	template<class Elem> static std::basic_string<Elem> ParseCSTokens(const std::basic_string<Elem>& str);
};

template<class Elem, class ...Args>
inline std::basic_string<Elem> StringTools::CSFormat(std::basic_string<Elem> fmt, Args ...args)
{
	

	return fmt;
}

template<class Elem, class Traits, class Allocator>
inline bool StringTools::IsEscaped(const std::basic_string<Elem, Traits, Allocator>& str, size_t offset, Elem escapeChar)
{
	return IsEscaped(std::basic_string_view<Elem, Traits>(str), offset, escapeChar);
}

template<class Elem, class Traits>
inline bool StringTools::IsEscaped(const std::basic_string_view<Elem, Traits>& str, size_t offset, Elem escapeChar)
{
	if (offset == 0)
		return false;

	if (offset >= str.size())
		throw std::domain_error(CSFormat("offset ({0}) was greater than string length ({1})"s, offset, str.size()));
	
	bool escaped = false;
	for (auto c = str.crbegin() - offset; c != str.crend(); c++)
	{
		if (*c == escapeChar)
			escaped = !escaped;
		else
			break;
	}

	return escaped;
}

template<class Elem>
inline std::basic_string<Elem> StringTools::ParseCSTokens(const std::basic_string<Elem>& str)
{
	return std::basic_string<Elem>();
}