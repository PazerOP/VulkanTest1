#include "stdafx.h"
#include "ShaderType.h"

template<> vk::ShaderStageFlagBits Enums::convert(ShaderType type)
{
	assert(Enums::validate(type));

	switch (type)
	{
	case ShaderType::Vertex:					return vk::ShaderStageFlagBits::eVertex;

	case ShaderType::TessellationControl:		return vk::ShaderStageFlagBits::eTessellationControl;
	case ShaderType::TessellationEvaluation:	return vk::ShaderStageFlagBits::eTessellationEvaluation;

	case ShaderType::Geometry:					return vk::ShaderStageFlagBits::eGeometry;

	case ShaderType::Compute:					return vk::ShaderStageFlagBits::eCompute;

	case ShaderType::Fragment:					return vk::ShaderStageFlagBits::eFragment;
	}

	assert(false);
	return vk::ShaderStageFlagBits();
}

std::ostream& operator<<(std::ostream& lhs, ShaderType rhs)
{
	assert(Enums::validate(rhs));

	switch (rhs)
	{
	case ShaderType::Vertex:					return lhs << "ShaderType::Vertex"sv;

	case ShaderType::TessellationControl:		return lhs << "ShaderType::TessellationControl"sv;
	case ShaderType::TessellationEvaluation:	return lhs << "ShaderType::TessellationEvaluation"sv;

	case ShaderType::Geometry:					return lhs << "ShaderType::Geometry"sv;

	case ShaderType::Compute:					return lhs << "ShaderType::Compute"sv;

	case ShaderType::Fragment:					return lhs << "ShaderType::Fragment"sv;
	}

	assert(false);
	return lhs;
}
