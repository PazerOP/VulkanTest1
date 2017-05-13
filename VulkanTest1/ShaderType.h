#pragma once
#include "Enums.h"

enum class ShaderType
{
	Vertex,
	TessellationControl,
	TessellationEvaluation,
	Gemoetry,
	Compute,
	Fragment,
	Pixel = Fragment,
};

template<> __forceinline constexpr auto Enums::min<ShaderType>() { return Enums::value(ShaderType::Vertex); }
template<> __forceinline constexpr auto Enums::max<ShaderType>() { return Enums::value(ShaderType::Pixel); }