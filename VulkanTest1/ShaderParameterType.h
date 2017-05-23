#pragma once
#include "BaseException.h"
#include "Enums.h"

#include <spirv_common.hpp>

using ShaderParameterType = spirv_cross::SPIRType::BaseType;

class InvalidShaderParameterTypeException : public BaseException<>
{
public:
	InvalidShaderParameterTypeException(const std::string& msg) : BaseException("InvalidShaderParameterTypeException"s, msg) { }

	InvalidShaderParameterTypeException(ShaderParameterType type) :
		InvalidShaderParameterTypeException(StringTools::CSFormat("Invalid ShaderParameterType {0}", Enums::value(type))) { }
};

class UnexpectedShaderParameterTypeException : public BaseException<>
{
public:
	UnexpectedShaderParameterTypeException(const std::string& msg) : BaseException("UnexpectedShaderParameterTypeException"s, msg) { }

	UnexpectedShaderParameterTypeException(ShaderParameterType type) :
		UnexpectedShaderParameterTypeException(StringTools::CSFormat("Unexpected ShaderParameterType {0}", type)) { }
};

template<> __forceinline constexpr auto Enums::min<ShaderParameterType>() { return Enums::value(ShaderParameterType::Void); }
template<> __forceinline constexpr auto Enums::max<ShaderParameterType>() { return Enums::value(ShaderParameterType::Sampler); }

extern std::ostream& operator<<(std::ostream& lhs, ShaderParameterType rhs);