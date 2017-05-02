#pragma once
#include <cassert>
#include <locale>
#include <string>
#include <string_view>
#include <vector>

#include "StringConverter.h"

using namespace std::string_literals;

// Missing user-defined literals for string_view, apparently should be fixed in VS 15.3
// https://developercommunity.visualstudio.com/comments/48859/view.html
#if _MSC_VER == 1910
#pragma warning(push)
#pragma warning(disable : 4455)	// literal suffix identifiers that do not start with an underscore are reserved
constexpr std::string_view operator "" sv(const char* str, size_t len) noexcept
{
	return std::basic_string_view<char>(str, len);
}
constexpr std::u16string_view operator "" sv(const char16_t* str, size_t len) noexcept
{
	return std::basic_string_view<char16_t>(str, len);
}
constexpr std::u32string_view operator "" sv(const char32_t* str, size_t len) noexcept
{
	return std::basic_string_view<char32_t>(str, len);
}
constexpr std::wstring_view operator "" sv(const wchar_t* str, size_t len) noexcept
{
	return std::basic_string_view<wchar_t>(str, len);
}
#pragma warning(pop)
#endif

inline std::string& to_string(std::string& str) { return str; }
inline std::string to_string(const std::string& str) { return str; }
inline std::string to_string(const char* str) { return std::string(str); }

class utf8_exception : public std::runtime_error
{
public:
	utf8_exception(const char* msg) : std::runtime_error(msg) { }
};

class StringTools final
{
public:
	StringTools() = delete;
	StringTools(const StringTools& other) = delete;
	StringTools(StringTools&& other) = delete;
	~StringTools() = delete;

	static void UnitTests();

	template<class... Args> static std::string CSFormat(const char* fmt, Args... args) { return CSFormat(std::string(fmt), args...); }
	static std::string CSFormat(const char* fmt) { return std::string(fmt); }
	template<class... Args> static std::string CSFormat(std::string fmt, Args... args);
	static std::string CSFormat(const std::string& fmt) { return fmt; }
	template<class... Args> static std::string CSFormat(const std::string_view& fmt, Args... args) { return CSFormat(std::string(fmt), args...); }
	static std::string CSFormat(const std::string_view& fmt) { return std::string(fmt); }

	template<class CharT> static bool IsEscaped(const CharT* str, size_t offset, CharT escapeChar = '\\');
	template<class CharT, class Traits, class Alloc> static bool IsEscaped(const std::basic_string<CharT, Traits, Alloc>& str, size_t offset, CharT escapeChar = '\\');
	template<class CharT, class Traits> static bool IsEscaped(const std::basic_string_view<CharT, Traits>& str, size_t offset, CharT escapeChar = '\\');

	// Determines the number of bytes in a utf8 character.
	static size_t UTF8Size(const char* ptr, const char* endPtr = nullptr);

	template<class T> static constexpr bool is_char_v =
		std::is_same_v<T, char> ||
		std::is_same_v<T, wchar_t> ||
		std::is_same_v<T, char16_t> ||
		std::is_same_v<T, char32_t>;

	/*template<class T1, class T2> static constexpr bool identical_numeric_types_v =
		sizeof(T1) == sizeof(T2) &&
		std::numeric_limits<T1>::min() == std::numeric_limits<T2>::min() &&
		std::numeric_limits<T1>::max() == std::numeric_limits<T2>::max();*/

private:
	struct CSToken
	{
		size_t m_FullTokenStart;
		size_t m_FullTokenEnd;

		size_t m_ID;

		size_t m_DataStart;
		size_t m_DataEnd;

		size_t m_ModeStart;
		size_t m_ModeEnd;
	};

	static std::vector<CSToken> ParseCSTokens(const std::string_view& str);
};

template<class... Args>
inline std::string StringTools::CSFormat(std::string fmt, Args... args)
{
	using namespace std;
	std::string strArgs[] = { to_string(args)... };

	constexpr size_t argCount = sizeof...(args);

	std::vector<CSToken> tokens = ParseCSTokens(fmt);

	for (auto tokenIter = tokens.crbegin(); tokenIter != tokens.crend(); tokenIter++)
	{
		// For now, we're lame
		// Just replace the tokens with the stringified args based on the tokens' ID
		const CSToken& token = *tokenIter;

		assert(token.m_ID < argCount);

		fmt.replace(fmt.begin() + token.m_FullTokenStart, fmt.begin() + token.m_FullTokenEnd, strArgs[token.m_ID]);
	}

	return fmt;
}

template<class CharT>
inline bool StringTools::IsEscaped(const CharT* str, size_t offset, CharT escapeChar)
{
	static_assert(is_char_v<CharT>, "Invalid char type!");

	return IsEscaped(std::basic_string_view<CharT>(str), offset, escapeChar);
}

template<class CharT, class Traits, class Alloc>
inline bool StringTools::IsEscaped(const std::basic_string<CharT, Traits, Alloc>& str, size_t offset, CharT escapeChar)
{
	return IsEscaped(std::basic_string_view<CharT, Traits>(str), offset, escapeChar);
}

template<class CharT, class Traits>
inline bool StringTools::IsEscaped(const std::basic_string_view<CharT, Traits>& str, size_t offset, CharT escapeChar)
{
	bool escaped = false;
	for (auto it = str.crend() - offset; it != str.crend(); ++it)
	{
		if (*it == escapeChar)
			escaped = !escaped;
		else
			break;
	}

	return escaped;
}

inline std::vector<StringTools::CSToken> StringTools::ParseCSTokens(const std::string_view& str)
{
	enum class GatherMode
	{
		Fresh,
		GatheringMode,
		GatheredMode,
		GatheredID,
		GatheringData,

		// Throw this CSToken away, it's malformed somehow
		Garbage,
	} gatherMode = GatherMode::Fresh;

	std::vector<CSToken> retVal;

	size_t i = 0;
	size_t braceLevel = 0;

	bool isEscaped = false;

	CSToken current;

	for (auto charIter = str.begin(); charIter != str.end(); charIter++)
	{
		const char c = *charIter;
		if (isEscaped)
			isEscaped = false;
		else if (c == '\\')
			isEscaped = true;
		else if (c == '{')
		{
			braceLevel++;

			if (braceLevel == 1)
			{
				gatherMode = GatherMode::Fresh;

				current.m_ID = 0;
				current.m_FullTokenStart = i;
				current.m_ModeStart = current.m_ModeEnd = 0;
				current.m_DataStart = current.m_DataEnd = 0;
			}
		}
		else if (c == ':' && braceLevel == 1)
		{
			if (gatherMode == GatherMode::Fresh || gatherMode == GatherMode::GatheredMode)
				gatherMode = GatherMode::Garbage;
			else
				gatherMode = GatherMode::GatheringData;

			current.m_DataStart = i + 1;
		}
		else if (c == '}')
		{
			assert(braceLevel >= 1);
			if (braceLevel >= 1)
				braceLevel--;

			if (!braceLevel)
			{
				if (gatherMode == GatherMode::GatheringData)
					current.m_DataEnd = i;
				else if (gatherMode == GatherMode::Fresh)
					gatherMode = GatherMode::Garbage;

				if (gatherMode != GatherMode::Garbage)
				{
					current.m_FullTokenEnd = i + 1;
					retVal.push_back(current);
				}
			}
		}
		else if (gatherMode == GatherMode::Fresh)
		{
			if (isdigit(c))
			{
				size_t charsRead;
				bool success;
				current.m_ID = StringConverter::From<size_t>(std::string_view(str.data() + i, str.size() - i), &charsRead, &success);

				if (!success)
					gatherMode = GatherMode::Garbage;
				else
				{
					gatherMode = GatherMode::GatheredID;
					charIter += charsRead;
					charIter -= 1;
				}
			}
			else
			{
				current.m_ModeStart = i;
				current.m_ModeEnd = 0;
				gatherMode = GatherMode::GatheringMode;
			}
		}

		i++;
	}

	return retVal;
}