#include "stdafx.h"
#include "ShaderType.h"

#pragma warning(push)
#pragma warning(disable : 4715)	// 'function' : not all control paths return a value
template<> vk::ShaderStageFlagBits Enums::convert(ShaderType type)
{
	assert(Enums::validate(type));

	switch (type)
	{
	case ShaderType::Vertex:					return vk::ShaderStageFlagBits::eVertex;

	case ShaderType::TessellationControl:		return vk::ShaderStageFlagBits::eTessellationControl;
	case ShaderType::TessellationEvaluation:	return vk::ShaderStageFlagBits::eTessellationEvaluation;

	case ShaderType::Gemoetry:					return vk::ShaderStageFlagBits::eGeometry;

	case ShaderType::Compute:					return vk::ShaderStageFlagBits::eCompute;

	case ShaderType::Fragment:					return vk::ShaderStageFlagBits::eFragment;
	}
}
#pragma warning(pop)