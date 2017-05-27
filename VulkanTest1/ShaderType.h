#pragma once
#include "Enums.h"

enum class ShaderType
{
	Vertex,
	TessellationControl,
	TessellationEvaluation,
	Geometry,
	Compute,
	Fragment,
	Pixel = Fragment,
};

template<> __forceinline constexpr auto Enums::min<ShaderType>() { return Enums::value(ShaderType::Vertex); }
template<> __forceinline constexpr auto Enums::max<ShaderType>() { return Enums::value(ShaderType::Pixel); }

extern std::ostream& operator<<(std::ostream& lhs, ShaderType rhs);