#pragma once
#include <string>

std::string vstrprintf(const char* fmt, va_list args);
std::string strprintf(const char* fmt, ...);