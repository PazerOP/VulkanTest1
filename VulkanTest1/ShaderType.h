#pragma once

enum class ShaderType
{
	Vertex,
	TessellationControl,
	TessellationEvaluation,
	Gemoetry,
	Compute,
	Fragment,
	Pixel = Fragment,

	Count,
};

__forceinline bool validate_enum_value(ShaderType value)
{
	return underlying_value(value) >= 0 && underlying_value(value) < underlying_value(ShaderType::Count);
}