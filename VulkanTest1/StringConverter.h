#pragma once
#include <cassert>
#include <cstdint>
#include <string>
#include <sstream>
#include <utility>

class StringConverter final
{
public:
	StringConverter() = delete;
	StringConverter(const StringConverter& other) = delete;
	StringConverter(StringConverter&& other) = delete;
	~StringConverter() = delete;

	template<class ValueT> static ValueT From(const std::string_view& str, size_t* charsRead = nullptr, bool* success = nullptr);

	static uint32_t ToUInt32(const std::string_view& str, size_t* charsRead = nullptr, bool* success = nullptr);
	static uint64_t ToUInt64(const std::string_view& str, size_t* charsRead = nullptr, bool* success = nullptr);
};

template<>
__forceinline uint32_t StringConverter::From<uint32_t>(const std::string_view& str, size_t* charsRead, bool* success)
{
	return ToUInt32(str, charsRead, success);
}

template<>
__forceinline uint64_t StringConverter::From<uint64_t>(const std::string_view& str, size_t* charsRead, bool* success)
{
	return ToUInt64(str, charsRead, success);
}

inline uint32_t StringConverter::ToUInt32(const std::string_view & str, size_t* charsRead, bool* success)
{
	uint32_t retVal;
	int read;

	if (charsRead)
		read = sscanf_s(str.data(), "%u%zn", &retVal, charsRead);
	else
		read = sscanf_s(str.data(), "%u", &retVal);

	assert(read == 0 || read == 1);
	if (success)
		*success = (read == 1);

	return retVal;
}

inline uint64_t StringConverter::ToUInt64(const std::string_view& str, size_t* charsRead, bool* success)
{
	uint64_t retVal;
	int read;

	if (charsRead)
		read = sscanf_s(str.data(), "%llu%zn", &retVal, charsRead);
	else
		read = sscanf_s(str.data(), "%llu", &retVal);

	assert(read == 0 || read == 1);
	if (success)
		*success = (read == 1);

	return retVal;
}