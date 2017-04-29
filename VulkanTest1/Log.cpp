#include "Log.h"

#include <clocale>
#include <codecvt>
#include "FixedWindows.h"
#include <locale>

void Log::MsgRaw(const std::string& str)
{
	if (!str.data())
		return;
	
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;	
	OutputDebugStringW(converter.from_bytes(str).c_str());
}

void Log::BlockMsgRaw(std::string str, size_t charsPerLine)
{
	if (!charsPerLine)
		throw std::invalid_argument("charsPerLine was 0");

	constexpr auto BLOCK_CHAR = u8"█"sv;
	constexpr auto START_BLOCK_CHARS = u8"\n████ "sv;
	constexpr auto END_BLOCK_CHARS = u8" ████\n"sv;
	constexpr auto NEWLINE_BLOCK_CHARS = u8" ████\n████ "sv;

	constexpr size_t ENCODING_ERROR = (size_t)-1;
	constexpr size_t INCOMPLETE_CHAR = (size_t)-2;

	enum class Wrap
	{
		None,
		Auto,
		User,
	} lastWrap = Wrap::None;

	size_t maxWidth = 0;

	str.insert(0, START_BLOCK_CHARS);

	const char* ptr = str.c_str();
	size_t charsThisLine = 0;					// UTF-8 characters
	size_t dataIdx = START_BLOCK_CHARS.size();	// Memory offset
	while (ptr[dataIdx])
	{
		if (ptr[dataIdx] == '\n')
		{
			str.replace(dataIdx, 1, NEWLINE_BLOCK_CHARS);
			ptr = str.c_str();
			dataIdx += NEWLINE_BLOCK_CHARS.size();
			charsThisLine = 0;
			lastWrap = Wrap::User;
			continue;
		}
		else if (charsThisLine >= charsPerLine)
		{
			str.insert(dataIdx, NEWLINE_BLOCK_CHARS);
			ptr = str.c_str();
			dataIdx += NEWLINE_BLOCK_CHARS.size();
			charsThisLine = 0;
			lastWrap = Wrap::Auto;
			continue;
		}

		if (lastWrap == Wrap::Auto && !charsThisLine && ptr[dataIdx] == ' ')
		{
			str.erase(dataIdx, 1);
			ptr = str.c_str();
		}

		lastWrap = Wrap::None;

		std::mbstate_t state = std::mbstate_t();
		const auto length = StringTools::UTF8Size(&ptr[dataIdx], &ptr[str.size()]);

		assert(length > 0);
		charsThisLine++;
		dataIdx += length;

		maxWidth = max(maxWidth, charsThisLine);
	}

	// Pad spaces until we reach maxwidth
	for (size_t i = 0; i < (maxWidth - charsThisLine); i++)
		str.insert(str.size(), " "sv);

	const size_t headerFooterWidth = maxWidth + 10;
	for (size_t i = 0; i < headerFooterWidth; i++)
		str.insert(0, BLOCK_CHAR);

	str.insert(str.size(), END_BLOCK_CHARS);

	for (size_t i = 0; i < headerFooterWidth; i++)
		str.insert(str.size(), BLOCK_CHAR);

	str.insert(str.size(), "\n"sv);

	MsgRaw(str);
}
