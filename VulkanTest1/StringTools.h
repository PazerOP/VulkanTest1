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

inline std::string to_string(const char* str) { return std::string(str); }
inline std::wstring to_wstring(const wchar_t* str) { return std::wstring(str); }

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

	template<class CharT, class Traits = std::char_traits<CharT>, class Alloc = std::allocator<CharT>, class... Args> static std::basic_string<CharT, Traits, Alloc> CSFormat(const CharT* fmt, Args... args);
	template<class CharT, class Traits = std::char_traits<CharT>, class Alloc = std::allocator<CharT>, class... Args> static std::basic_string<CharT, Traits, Alloc> CSFormat(std::basic_string<CharT, Traits, Alloc> fmt, Args... args);

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
	template<class CharT, class Traits> struct CSToken
	{
		size_t m_FullTokenStart;
		size_t m_FullTokenEnd;

		size_t m_ID;

		size_t m_DataStart;
		size_t m_DataEnd;

		size_t m_ModeStart;
		size_t m_ModeEnd;
	};

	template<class CharT, class Traits> static std::vector<CSToken<CharT, Traits>> ParseCSTokens(const std::basic_string_view<CharT, Traits>& str);
};

template<class CharT, class Traits, class Alloc, class... Args>
inline std::basic_string<CharT, Traits, Alloc> StringTools::CSFormat(const CharT* fmt, Args... args)
{
	return CSFormat(std::basic_string<CharT, Traits, Alloc>(fmt), args...);
}

template<class CharT, class Traits, class Alloc, class... Args>
inline std::basic_string<CharT, Traits, Alloc> StringTools::CSFormat(std::basic_string<CharT, Traits, Alloc> fmt, Args... args)
{
	using StringType = std::basic_string<CharT, Traits, Alloc>;

	using namespace std;

	StringType strArgs[] = { ToString(args)... };

	constexpr size_t argCount = sizeof(strArgs) / sizeof(strArgs[0]);

	std::vector<CSToken<CharT, Traits>> tokens = ParseCSTokens(std::basic_string_view<CharT, Traits>(fmt));

	for (auto tokenIter = tokens.crbegin(); tokenIter != tokens.crend(); tokenIter++)
	{
		// For now, we're lame
		// Just replace the tokens with the stringified args based on the tokens' ID
		const CSToken<CharT, Traits>& token = *tokenIter;

		assert(token.m_ID < argCount);

		fmt.replace(fmt.begin() + token.m_FullTokenStart, fmt.begin() + token.m_FullTokenEnd, strArgs[token.m_ID]);
	}

	return fmt;
}

template<class Traits, class Alloc>
inline std::basic_string<char32_t, Traits, Alloc> StringTools::ToU32String(const std::string_view& str)
{
	std::basic_string<char32_t, Traits, Alloc> retVal(str.size(), '\0');
	//retVal.reserve(str.size());

	auto& f = std::use_facet<std::codecvt<uint32_t, char, std::mbstate_t>>(std::locale());

	std::mbstate_t state = {};
	const char* from_next;
	uint32_t* to_next;
	f.in(state,
		&str[0], &str[str.size() - 1] + 1, from_next,
		(uint32_t*)&retVal[0], ((uint32_t*)&retVal[retVal.size() - 1]) + 1, to_next);

	return retVal;
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

template<class CharT, class Traits>
std::vector<StringTools::CSToken<CharT, Traits>> StringTools::ParseCSTokens(const std::basic_string_view<CharT, Traits>& str)
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

	std::vector<CSToken<CharT, Traits>> retVal;

	size_t i = 0;
	size_t braceLevel = 0;

	bool isEscaped = false;

	CSToken<CharT, Traits> current;

	for (auto charIter = str.begin(); charIter != str.end(); charIter++)
	{
		const CharT c = *charIter;
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
				current.m_ID = FromString<size_t>(std::basic_string_view<CharT, Traits>(str.data() + i), &charsRead, &success);

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