#include "stdafx.h"
#include "ShaderParameterType.h"

std::ostream& operator<<(std::ostream& lhs, ShaderParameterType rhs)
{
	switch (rhs)
	{
	case ShaderParameterType::AtomicCounter:	return lhs << "spirv_cross::SPIRType::BaseType::AtomicCounter"sv;
	case ShaderParameterType::Boolean:			return lhs << "spirv_cross::SPIRType::BaseType::Boolean"sv;
	case ShaderParameterType::Char:				return lhs << "spirv_cross::SPIRType::BaseType::Char"sv;
	case ShaderParameterType::Double:			return lhs << "spirv_cross::SPIRType::BaseType::Double"sv;
	case ShaderParameterType::Float:			return lhs << "spirv_cross::SPIRType::BaseType::Float"sv;
	case ShaderParameterType::Image:			return lhs << "spirv_cross::SPIRType::BaseType::Image"sv;
	case ShaderParameterType::Int:				return lhs << "spirv_cross::SPIRType::BaseType::Int"sv;
	case ShaderParameterType::Int64:			return lhs << "spirv_cross::SPIRType::BaseType::Int64"sv;
	case ShaderParameterType::SampledImage:		return lhs << "spirv_cross::SPIRType::BaseType::SampledImage"sv;
	case ShaderParameterType::Sampler:			return lhs << "spirv_cross::SPIRType::BaseType::Sampler"sv;
	case ShaderParameterType::Struct:			return lhs << "spirv_cross::SPIRType::BaseType::Struct"sv;
	case ShaderParameterType::UInt:				return lhs << "spirv_cross::SPIRType::BaseType::UInt"sv;
	case ShaderParameterType::UInt64:			return lhs << "spirv_cross::SPIRType::BaseType::UInt64"sv;
	case ShaderParameterType::Unknown:			return lhs << "spirv_cross::SPIRType::BaseType::Unknown"sv;
	case ShaderParameterType::Void:				return lhs << "spirv_cross::SPIRType::BaseType::Void"sv;
	}

	throw InvalidShaderParameterTypeException(rhs);
}