#include "stdafx.h"
#include "StringTools.h"

#include <cassert>
#include <cuchar>
#include <sstream>

void StringTools::UnitTests()
{
	const char testString[] = u8"\tLayer: \\\\\\{0} (v{1}) -\\🔥 {2}";
	//const auto const testSubStr = testString.substr();
	assert(!IsEscaped(testString, sizeof(testString) / sizeof(testString[0]) - 1));
	assert(!IsEscaped(testString, 8));
	assert(IsEscaped(testString, 9));
	assert(!IsEscaped(testString, 10));
	assert(IsEscaped(testString, 11));
	assert(!IsEscaped(testString, 23));
	assert(IsEscaped(testString, 24));
}

// See https://en.wikipedia.org/wiki/UTF-8#Description
size_t StringTools::UTF8Size(const char* ptr, const char* endPtr)
{
	if (!ptr)
		throw std::invalid_argument("\"ptr\" was null.");

	const uint8_t* const data = reinterpret_cast<const uint8_t*>(ptr);

	const size_t maxBytes = endPtr ? endPtr - ptr : (size_t)-1;

	if (!*ptr)
		return 0;
	if ((data[0] & 0xC0) == 0x80)
		throw utf8_exception("Started in the middle of a character");
	if (!(data[0] & 0x80))
		return 1;
	if (maxBytes >= 2 && (data[0] & 0xE0) == 0xC0)
	{
		if ((data[1] & 0xC0) != 0x80)
			throw utf8_exception("Excpected a 2-byte sequence, but second byte was malformed");

		return 2;
	}
	if (maxBytes >= 3 && (data[0] & 0xF0) == 0xE0)
	{
		if ((data[1] & 0xC0) != 0x80 || (data[2] & 0xC0) != 0x80)
			throw utf8_exception("Expected a 3-byte sequence, but byte 2 or 3 were malformed");

		return 3;
	}
	if (maxBytes >= 4 && (data[0] & 0xF8) == 0xF0)
	{
		if ((data[1] & 0xC0) != 0x80 || (data[2] & 0xC0) != 0x80 || (data[3] & 0xC0) != 0x80)
			throw utf8_exception("Excpected a 4-byte sequence, but byte 2, 3, or 4 were malformed");

		return 4;
	}

	throw utf8_exception("Malformed utf-8 byte");
}

std::vector<StringTools::CSToken> StringTools::ParseCSTokens(const std::string_view& str)
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
		const char& c = *charIter;
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
				assert(&c == (str.data() + i));
				size_t charsRead;
				bool success;
				current.m_ID = StringConverter::From<size_t>(std::string_view(str.data() + i, str.size() - i), &charsRead, &success);

				if (!success)
					gatherMode = GatherMode::Garbage;
				else
				{
					gatherMode = GatherMode::GatheredID;
					charIter += charsRead;
					i += charsRead;
					charIter -= 1;
					i -= 1;
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

#if 0
bool StringTools::IsEscaped(const char* str, size_t offset, char escapeChar)
{
	if (offset == 0)
		return false;

	size_t escapeCount = 0;
	size_t i = 0;
	std::mbstate_t state = {};
	char32_t c;

	while (true)
	{
		const size_t bytes = std::mbrtoc32(&c, str, strnlen(str, 4), &state);
		if (!bytes)
			throw std::domain_error(CSFormat("offset ({0}) was greater than string length ({1})"s, offset, i));

		if (i == offset)
			return !!(escapeCount % 2);

		if (c == escapeChar)
			escapeCount++;
		else
			escapeCount = 0;

		i++;
		str += bytes;
	}

	return false;
}
#endif

std::string to_string(const vk::Extent2D& extent2D)
{
	return StringTools::CSFormat("({0}x{1})", extent2D.width, extent2D.height);
}

std::string to_string(const glm::mat4& mat4)
{
	return StringTools::CSFormat(
		"[{0} {1} {2} {3}]\n"
		"[{4} {5} {6} {7}]\n"
		"[{8} {9} {10} {11}]\n"
		"[{12} {13} {14} {15}]",
		mat4[0][0], mat4[1][0], mat4[2][0], mat4[3][0],
		mat4[0][1], mat4[1][1], mat4[2][1], mat4[3][1],
		mat4[0][2], mat4[1][2], mat4[2][2], mat4[3][2],
		mat4[0][3], mat4[1][3], mat4[2][3], mat4[3][3]);
}
