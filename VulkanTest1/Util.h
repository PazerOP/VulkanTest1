#pragma once

#ifdef _DEBUG
#include <cassert>
#define AssertAR(before, statement, after)	assert(before statement after)
#else
#define AssertAR(before, statement, after)	(statement)
#endif