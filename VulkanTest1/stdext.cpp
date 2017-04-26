#include "stdext.h"

#include <cassert>
#include <cstdarg>

std::string vstrprintf(const char* fmt, va_list args)
{
	va_list args2;
	va_copy(args2, args);

	const auto length = vsnprintf(nullptr, 0, fmt, args) + 1;

	assert(length > 0);
	if (length <= 0)
		return nullptr;

	std::string retVal;
	retVal.resize(length);
	const auto written = vsnprintf(&retVal[0], length, fmt, args2);

	// Remove the null terminator
	retVal.resize(length - 1);

	assert((written + 1) == length);

	return retVal;
}

std::string strprintf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	return vstrprintf(fmt, args);
}
