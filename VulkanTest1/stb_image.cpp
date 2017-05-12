#include "stdafx.h"

#include <mutex>
#include <set>

static std::mutex s_AllocsMutex;
static std::set<void*> s_Allocs;

static void* AllocFn(size_t size)
{
	std::lock_guard<decltype(s_AllocsMutex)> lock(s_AllocsMutex);

	void* retVal = malloc(size);

	assert(s_Allocs.find(retVal) == s_Allocs.end());
	s_Allocs.insert(retVal);

	return retVal;
}
static void* ReallocFn(void* old, size_t newSize)
{
	std::lock_guard<decltype(s_AllocsMutex)> lock(s_AllocsMutex);

	void* retVal = realloc(old, newSize);
	assert(s_Allocs.find(old) != s_Allocs.end());
	s_Allocs.erase(old);

	assert(s_Allocs.find(retVal) == s_Allocs.end());
	s_Allocs.insert(retVal);
	return retVal;
}
static void FreeFn(void* p)
{
	std::lock_guard<decltype(s_AllocsMutex)> lock(s_AllocsMutex);

	free(p);

	assert(s_Allocs.find(p) != s_Allocs.end());
	s_Allocs.erase(p);
}

#define STBI_MALLOC(sz)			AllocFn(sz)
#define STBI_REALLOC(p, newsz)	ReallocFn(p, newsz)
#define STBI_FREE(p)			FreeFn(p)

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"