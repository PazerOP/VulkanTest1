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

	struct InputConstant
	{
		uint32_t m_BindingID;
		spirv_cross::SPIRType m_Type;
		std::string m_FriendlyName;
		std::string m_FullName;

		enum class Decoration
		{
			None = 0,
			Parameter = (1 << 0),
			Input = (1 << 1),
			InputIndex = (1 << 2),
			Output = (1 << 3),
			OutputIndex = (1 << 4),
			TexMode = (1 << 5),
		} m_Decoration;

		void ParseFullName();
	};

	struct BasicType
	{
		std::string m_FriendlyName;
		std::string m_FullName;
		spirv_cross::SPIRType m_Type;

		std::multimap<std::string, BasicType> m_Members;

		enum class Decoration
		{
			None = 0,
			Parameter = (1 << 0),
			Tex1D = (1 << 1),
			Tex2D = (1 << 2),
			Tex3D = (1 << 3),
		} m_Decoration;

		void ParseFullName();
	};

	struct InputVariable : public BasicType
	{
		uint32_t m_SetID;
		uint32_t m_BindingID;
		vk::DescriptorType m_DescriptorType;
	};

	struct InputTexture
	{
		std::vector<InputVariable> m_Dimensions;
	};

	ShaderType GetType() const { return m_Type; }
	const auto& GetPath() const { return m_Path; }
	const auto& GetName() const { return m_Name; }
	const auto& GetCodeBytes() const { return m_CodeBytes; }

	const auto& GetInputVariables() const { return m_InputVariables; }
	const auto& GetInputSpecConstants() const { return m_InputConstants; }
	const auto& GetInputTextures() const { return m_InputTextures; }

	const bool HasInputFriendly(const std::string& friendly) const;

private:
	void LoadShaderType(const spirv_cross::Compiler& spirvComp);
	void LoadInputParams(const spirv_cross::Compiler& spirvComp);
	void LoadSpecConstants(const spirv_cross::Compiler& spirvComp);

	static const std::string PREFIX_PARAMETER;
	static const std::string PREFIX_INPUT;
	static const std::string PREFIX_INPUTINDEX;
	static const std::string PREFIX_OUTPUT;
	static const std::string PREFIX_OUTPUTINDEX;
	static const std::string PREFIX_TEXMODE;
	static const std::string PREFIX_TEX1D;
	static const std::string PREFIX_TEX2D;
	static const std::string PREFIX_TEX3D;

	std::pair<std::vector<uint32_t>, size_t> m_CodeBytes;

	std::map<std::string, InputVariable> m_InputVariables;		// Uniforms/whatever
	std::map<std::string, InputTexture> m_InputTextures;		// Uniform sampler1D/2D/3D
	std::map<std::string, InputConstant> m_InputConstants;		// Specialization constants

	ShaderType m_Type;
	std::filesystem::path m_Path;
	std::string m_Name;
};