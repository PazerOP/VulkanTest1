#pragma once

#ifdef _DEBUG
#include <cassert>
#define AssertAR(before, statement, after)	assert(before statement after)
#else
#define AssertAR(before, statement, after)	(statement)
#endif

#undef min
#undef max

template<class T>
inline constexpr auto underlying_value(const T& in)
{
	return std::underlying_type_t<T>(in);
}

template<class TOut, class TIn>
inline TOut overflow_check(const TIn& in)
{
	assert(in >= std::numeric_limits<TOut>::min());
	assert(in <= std::numeric_limits<TOut>::max());
	return (TOut)in;
}