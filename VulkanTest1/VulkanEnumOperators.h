#pragma once

#define VULKAN_ENUM_OPERATORS_SET(ctype, cpptype, op)													\
	inline cpptype operator ## op ## (const ctype& lhs, const cpptype& rhs)								\
	{																									\
		return (cpptype)((std::underlying_type_t<ctype>)lhs op (std::underlying_type_t<cpptype>)rhs);	\
	}																									\
	inline cpptype operator ## op ## (const cpptype& lhs, const ctype& rhs)								\
	{																									\
		return (cpptype)((std::underlying_type_t<cpptype>)lhs op (std::underlying_type_t<ctype>)rhs);	\
	}																									\
	inline cpptype operator ## op ## (const cpptype& lhs, const cpptype& rhs)							\
	{																									\
		return (cpptype)((std::underlying_type_t<cpptype>)lhs op (std::underlying_type_t<cpptype>)rhs);	\
	}																									\


// Provides operators between the C and C++ enums present in vulkan.
#define VULKAN_ENUM_OPERATORS(ctype, cpptype)			\
	VULKAN_ENUM_OPERATORS_SET(ctype, cpptype, |)		\
	VULKAN_ENUM_OPERATORS_SET(ctype, cpptype, &)		\
														\
	inline bool operator!(const cpptype& e)				\
	{													\
		return !(std::underlying_type_t<cpptype>(e));	\
	}