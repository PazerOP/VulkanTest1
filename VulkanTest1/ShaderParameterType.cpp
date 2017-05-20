#include "stdafx.h"
#include "ShaderParameterType.h"

#pragma warning(push)
#pragma warning(error : 4062)
std::ostream& operator<<(std::ostream& lhs, ShaderParameterType rhs)
{
	switch (rhs)
	{
	case ShaderParameterType::Texture:	return lhs << "ShaderParameterType::Texture"sv;
	case ShaderParameterType::Float:	return lhs << "ShaderParameterType::Float"sv;
	case ShaderParameterType::Int:		return lhs << "ShaderParameterType::Int"sv;
	case ShaderParameterType::Bool:		return lhs << "ShaderParameterType::Bool"sv;
	}

	throw std::out_of_range(StringTools::CSFormat("Unexpected ShaderParameterType {0}", Enums::value(rhs)));
}
#pragma warning(pop)