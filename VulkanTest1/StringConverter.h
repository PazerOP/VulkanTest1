#pragma once
#include <cassert>
#include <cstdint>
#include <string>
#include <sstream>
#include <utility>

template<class ValueT>
class ToStringConverter
{
public:
	template<class CharT = char, class Traits = std::char_traits<CharT>, class Alloc = std::allocator<CharT>> static std::basic_string<CharT, Traits, Alloc> ToStr(const ValueT& value)
	{
		std::basic_stringstream<CharT, Traits, Alloc> retVal;
		retVal << value;
		return retVal.str();
	}
	//template<class StringT> static ValueT FromStr(const StringT* str, StringT** str_end);
};

template<class ValueT>
class FromStringConverter/*
{
public:
	template<class CharT, class Traits> static ValueT FromStr(const std::basic_string_view<CharT, Traits>& str);
}*/;

template<>
class FromStringConverter<uint32_t>
{
public:
	template<class CharT, class Traits> static uint32_t FromStr(const std::basic_string_view<CharT, Traits>& str, size_t* charsRead = nullptr, bool* success = nullptr)
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
};

template<>
class FromStringConverter<uint64_t>
{
public:
	template<class CharT, class Traits> static uint64_t FromStr(const std::basic_string_view<CharT, Traits>& str, size_t* charsRead = nullptr, bool* success = nullptr)
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
};

template<class ValueT, class CharT = char, class Traits = std::char_traits<CharT>, class Alloc = std::allocator<CharT>>
inline std::basic_string<CharT, Traits, Alloc> ToString(const ValueT& value)
{
	return ToStringConverter<ValueT>::ToStr<CharT, Traits, Alloc>(value);
}

template<class ValueT, class CharT, class Traits>
inline ValueT FromString(const std::basic_string_view<CharT, Traits>& str, size_t* charsRead = nullptr, bool* success = nullptr)
{
	return FromStringConverter<ValueT>::FromStr<CharT, Traits>(str, charsRead, success);
}