#include "StringTools.h"

#include <cassert>
#include <cuchar>

void StringTools::UnitTests()
{
	const auto const testString = u8"\tLayer: \\\\\\{0} (v{1}) -\\🔥 {2}"s;
	//const auto const testSubStr = testString.substr();
	assert(!IsEscaped(testString, testString.size() - 1));
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

bool StringTools::IsEscaped(const std::string_view& str, size_t offset, char32_t escapeChar)
{
	if (offset >= str.size())
		throw std::domain_error(CSFormat("offset ({0}) was greater than the string view length ({1})", offset, str.size()));

	return IsEscaped(str.data(), offset, escapeChar);
}

std::vector<StringTools::CSToken> StringTools::ParseCSTokens(const std::string_view& str)
{
	std::vector<CSToken> retVal;

	std::mbstate_t state = {};
	size_t lastNonEscapeChar = 0;
	size_t lastNonEscapeByte = 0;
	size_t charIndex = 0;
	char32_t c;

	for (size_t byteIndex = 0; byteIndex < str.size(); )
	{
		const size_t bytes = std::mbrtoc32(&c, &str[byteIndex], strnlen(&str[byteIndex], 4), &state);
		if (!bytes)
		{
			byteIndex++;
			charIndex++;
			continue;
		}

		const bool escaped = IsEscaped(str.substr(lastNonEscapeByte), byteIndex - lastNonEscapeByte);


		if (c == '{')
	}

	return retVal;
}
