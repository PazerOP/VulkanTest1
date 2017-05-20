#pragma once
#include "Enums.h"

enum class ShaderParameterType
{
	Texture,
	Float,
	Int,
	Bool,
};

template<> __forceinline constexpr auto Enums::min<ShaderParameterType>() { return Enums::value(ShaderParameterType::Texture); }
template<> __forceinline constexpr auto Enums::max<ShaderParameterType>() { return Enums::value(ShaderParameterType::Bool); }

extern std::ostream& operator<<(std::ostream& lhs, ShaderParameterType rhs);