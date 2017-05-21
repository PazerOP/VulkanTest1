#pragma once
#include "BaseException.h"
#include "ShaderParameterType.h"
#include "ShaderType.h"

#include <spirv_common.hpp>

#include <filesystem>

namespace spirv_cross
{
	class Compiler;
}

class ShaderModuleData
{
public:
	ShaderModuleData(const std::filesystem::path& path);

	class InvalidShaderTypeException : public BaseException<>
	{
	public:
		InvalidShaderTypeException(const std::string& msg) : BaseException("InvalidShaderTypeException"s, msg) { }
	};

	// Specialization Constant
	struct SpecConstant
	{
		std::string m_Name;
		uint32_t m_BindingID;
		spirv_cross::SPIRType m_Type;
	};

	const std::filesystem::path& GetPath() const { return m_Path; }
	const std::vector<uint32_t>& GetCodeBytes() const { return m_CodeBytes; }

	const std::vector<SpecConstant>& GetSpecConstants() const { return m_SpecConstants; }
	const SpecConstant* FindTexModeConstant(const std::string& name) const;

private:
	void LoadShaderType(const spirv_cross::Compiler& spirvComp);
	void LoadSpecConstants(const spirv_cross::Compiler& spirvComp);

	std::vector<uint32_t> m_CodeBytes;

	std::vector<SpecConstant> m_SpecConstants;

	ShaderType m_Type;
	std::filesystem::path m_Path;
};