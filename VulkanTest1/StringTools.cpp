#include "StringTools.h"

#include <cassert>
#include <cuchar>

void StringTools::UnitTests()
{
	const auto const testString = u8"\tLayer: \\\\\\{0} (v{1}) -\\🔥 {2}";
	//const auto const testSubStr = testString.substr();
	assert(!IsEscaped(testString, strlen(testString) - 1));
	assert(!IsEscaped(testString, 8));
	assert(IsEscaped(testString, 9));
	assert(!IsEscaped(testString, 10));
	assert(IsEscaped(testString, 11));
	assert(!IsEscaped(testString, 23));
	assert(IsEscaped(testString, 24));
}

bool StringTools::IsEscaped(const char* str, size_t offset, char32_t escapeChar)
{
	if (offset == 0)
		return false;

	//size_t len = lengthFn(str, offset + 1);
	//if (len <= offset)
	//	throw std::domain_error(CSFormat("offset ({0}) was greater than string length ({1})"s, offset, len));

	size_t escapeCount = 0;
	size_t i = 0;
	std::mbstate_t state = {};
	char32_t c;

	while (true)
	{
		size_t bytes = std::mbrtoc32(&c, str, strnlen(str, 4), &state);
		if (!bytes)
			throw std::domain_error(CSFormat("offset ({0}) was greater than string length"s, offset));

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