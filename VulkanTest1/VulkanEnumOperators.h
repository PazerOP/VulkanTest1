#pragma once
#include "Util.h"

#define VULKAN_ENUM_OPERATORS_SET(ctype, cpptype, op)								\
	inline cpptype operator ## op ## (const ctype& lhs, const cpptype& rhs)			\
	{																				\
		return cpptype(underlying_value(lhs) op underlying_value(rhs));				\
	}																				\
	inline cpptype operator ## op ## (const cpptype& lhs, const ctype& rhs)			\
	{																				\
		return cpptype(underlying_value(lhs) op underlying_value(rhs));				\
	}																				\
	inline cpptype operator ## op ## (const cpptype& lhs, const cpptype& rhs)		\
	{																				\
		return cpptype(underlying_value(lhs) op underlying_value(rhs));				\
	}																				\
																					\
	inline cpptype& operator ## op ## = ## (ctype& lhs, const cpptype& rhs)			\
	{																				\
		lhs = ctype(underlying_value(lhs) op underlying_value(rhs));				\
		return (cpptype&)(lhs);														\
	}																				\
	inline cpptype& operator ## op ## = ## (cpptype& lhs, const ctype& rhs)			\
	{																				\
		lhs = cpptype(underlying_value(lhs) op underlying_value(rhs));				\
		return lhs;																	\
	}																				\
	inline cpptype& operator ## op ## = ## (cpptype& lhs, const cpptype& rhs)		\
	{																				\
		lhs = cpptype(underlying_value(lhs) op underlying_value(rhs));				\
		return lhs;																	\
	}

// Provides operators between the C and C++ enums present in vulkan.
#define VULKAN_ENUM_OPERATORS(ctype, cpptype)			\
	VULKAN_ENUM_OPERATORS_SET(ctype, cpptype, |)		\
	VULKAN_ENUM_OPERATORS_SET(ctype, cpptype, &)		\
														\
	inline bool operator!(const cpptype& e)				\
	{													\
		return !(underlying_value(e));					\
	}