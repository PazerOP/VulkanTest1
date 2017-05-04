#pragma once

#ifdef _DEBUG
#include <cassert>
#define AssertAR(before, statement, after)	assert(before statement after)
#else
#define AssertAR(before, statement, after)	(statement)
#endif

#undef min
#undef max

#define SV_MACRO(x)	std::string_view(x, std::size(x))

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

class programmer_error : public std::logic_error
{
public:
	programmer_error(const char* msg) : std::logic_error(msg) { }
	programmer_error(const std::string& msg) : std::logic_error(msg) { }
};

class not_implemented_error : public programmer_error
{
public:
	not_implemented_error() : programmer_error("function not implemented!") { }
	not_implemented_error(const char* msg) : programmer_error(msg) { }
	not_implemented_error(const std::string& msg) : programmer_error(msg) { }
};

// std::make_array is still missing from from the stl??
template<class T, class... Args>
constexpr std::array<T, sizeof...(Args)> make_array(Args&&... t)
{
	return { std::forward<Args>(t)... };
}