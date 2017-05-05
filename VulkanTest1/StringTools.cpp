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