#pragma once
#include "BaseException.h"
#include "ShaderParameterType.h"
#include "ShaderType.h"

#include <spirv_common.hpp>

#include <filesystem>
#include <map>

namespace spirv_cross
{
	class Compiler;
}

// Represents metadata that has been reflected from the actual SPIR-V module.
class ShaderModuleData
{
public:
	ShaderModuleData(const std::filesystem::path& path);

	class InvalidShaderTypeException : public BaseException<>
	{
	public:
		InvalidShaderTypeException(const std::string& msg) : BaseException("InvalidShaderTypeException"s, msg) { }
	};

	struct InputParam
	{
		uint32_t m_BindingID;
		spirv_cross::SPIRType m_Type;
	};

	ShaderType GetType() const { return m_Type; }
	const std::filesystem::path& GetPath() const { return m_Path; }
	const std::vector<uint32_t>& GetCodeBytes() const { return m_CodeBytes; }

	const auto& GetInputParams() const { return m_InputParams; }
	const auto& GetSpecConstants() const { return m_SpecConstants; }

private:
	void LoadShaderType(const spirv_cross::Compiler& spirvComp);
	void LoadSpecConstants(const spirv_cross::Compiler& spirvComp);

	std::vector<uint32_t> m_CodeBytes;

	std::map<std::string, InputParam> m_InputParams;
	std::map<std::string, InputParam> m_SpecConstants;

	ShaderType m_Type;
	std::filesystem::path m_Path;
};