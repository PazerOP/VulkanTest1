#pragma once
#include "Enums.h"

enum class ShaderParameterType
{
	Texture,
	UniformBuffer,
};

template<> __forceinline constexpr auto Enums::min<ShaderParameterType>() { return Enums::value(ShaderParameterType::Texture); }
template<> __forceinline constexpr auto Enums::max<ShaderParameterType>() { return Enums::value(ShaderParameterType::UniformBuffer); }