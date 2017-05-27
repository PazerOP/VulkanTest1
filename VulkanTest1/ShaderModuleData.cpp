#include "stdafx.h"
#include "ShaderModuleData.h"

#include "ContentPaths.h"

#include <spirv_cross.hpp>

#include <fstream>
#include <map>

const std::string ShaderModuleData::PREFIX_PARAMETER = "_param"s;
const std::string ShaderModuleData::PREFIX_INPUT = "_input"s;
const std::string ShaderModuleData::PREFIX_INPUTINDEX = "_inputIndex"s;
const std::string ShaderModuleData::PREFIX_OUTPUT = "_output"s;
const std::string ShaderModuleData::PREFIX_OUTPUTINDEX = "_outputIndex"s;
const std::string ShaderModuleData::PREFIX_TEXMODE = "_texMode"s;
const std::string ShaderModuleData::PREFIX_TEX1D = "_tex1D"s;
const std::string ShaderModuleData::PREFIX_TEX2D = "_tex2D"s;
const std::string ShaderModuleData::PREFIX_TEX3D = "_tex3D"s;

ShaderModuleData::ShaderModuleData(const std::filesystem::path& path) :
	m_Path(path),
	m_Name(name_from_path(ContentPaths::Shaders(), path, false))
{
	std::ifstream stream(path.string(), std::ios::ate | std::ios::binary);

	if (!stream.is_open())
		throw std::runtime_error(StringTools::CSFormat("Failed to open file \"{0}\"", path));

	const size_t fileSize = stream.tellg();
	m_CodeBytes.first = std::vector<uint32_t>((fileSize + sizeof(uint32_t) - 1) / sizeof(uint32_t));
	m_CodeBytes.second = fileSize;
	//m_CodeBytes.first.back() = 0;
	stream.seekg(0);
	stream.read((char*)m_CodeBytes.first.data(), fileSize);

	spirv_cross::Compiler compiler(m_CodeBytes.first);
	LoadShaderType(compiler);
	LoadInputParams(compiler);
	LoadSpecConstants(compiler);
}

const bool ShaderModuleData::HasInputFriendly(const std::string& friendly) const
{
	return (m_InputConstants.find(friendly) != m_InputConstants.end() ||
			m_InputTextures.find(friendly) != m_InputTextures.end() ||
			m_InputVariables.find(friendly) != m_InputVariables.end());
}

void ShaderModuleData::LoadShaderType(const spirv_cross::Compiler& spirvComp)
{
	switch (spirvComp.get_execution_model())
	{
	case spv::ExecutionModel::ExecutionModelVertex:
		m_Type = ShaderType::Vertex;
		break;
	case spv::ExecutionModel::ExecutionModelTessellationControl:
		m_Type = ShaderType::TessellationControl;
		break;
	case spv::ExecutionModel::ExecutionModelTessellationEvaluation:
		m_Type = ShaderType::TessellationEvaluation;
		break;
	case spv::ExecutionModel::ExecutionModelGeometry:
		m_Type = ShaderType::Geometry;
		break;
	case spv::ExecutionModel::ExecutionModelFragment:
		m_Type = ShaderType::Fragment;
		break;
	case spv::ExecutionModel::ExecutionModelGLCompute:
		m_Type = ShaderType::Compute;
		break;

	default:
		throw InvalidShaderTypeException(StringTools::CSFormat("Unexpected shader execution model {0} encountered in \"{1}\"", Enums::value(spirvComp.get_execution_model()), m_Path));
	}
}

void ShaderModuleData::LoadInputParams(const spirv_cross::Compiler& spirvComp)
{
	m_InputVariables.clear();

	const auto& resources = spirvComp.get_shader_resources();
	for (const auto& inputParam : resources.uniform_buffers)
	{
		InputVariable newParam;
		newParam.m_SetID = spirvComp.get_decoration(inputParam.id, spv::Decoration::DecorationDescriptorSet);
		newParam.m_BindingID = spirvComp.get_decoration(inputParam.id, spv::Decoration::DecorationBinding);
		newParam.m_Type = spirvComp.get_type(inputParam.base_type_id);
		newParam.m_DescriptorType = vk::DescriptorType::eUniformBuffer;
		newParam.m_FullName = inputParam.name;

		for (size_t i = 0; i < newParam.m_Type.member_types.size(); i++)
		{
			const auto& memberType = spirvComp.get_type(newParam.m_Type.member_types[i]);

			BasicType member;
			member.m_Type = memberType;

			member.m_FullName = spirvComp.get_member_name(inputParam.base_type_id, i);
			member.ParseFullName();

			newParam.m_Members.insert(std::make_pair(member.m_FriendlyName, member));
		}

		AssertAR(, m_InputVariables.insert(std::make_pair(newParam.m_FullName, newParam)), .second);
	}

	for (const auto& inputTexture : resources.sampled_images)
	{
		InputVariable newParam;
		newParam.m_SetID = spirvComp.get_decoration(inputTexture.id, spv::Decoration::DecorationDescriptorSet);
		newParam.m_BindingID = spirvComp.get_decoration(inputTexture.id, spv::Decoration::DecorationBinding);
		newParam.m_Type = spirvComp.get_type(inputTexture.base_type_id);
		newParam.m_DescriptorType = vk::DescriptorType::eCombinedImageSampler;

		newParam.m_FullName = inputTexture.name;
		newParam.ParseFullName();

		const auto found = m_InputTextures.find(newParam.m_FriendlyName);
		if (found != m_InputTextures.end())
		{
			found->second.m_Dimensions.push_back(newParam);
		}
		else
		{
			InputTexture newTexture;
			newTexture.m_Dimensions.push_back(newParam);
			AssertAR(, m_InputTextures.insert(std::make_pair(newParam.m_FriendlyName, newTexture)), .second);
		}
	}
}

void ShaderModuleData::LoadSpecConstants(const spirv_cross::Compiler& spirvComp)
{
	m_InputConstants.clear();

	for (const auto& specConst : spirvComp.get_specialization_constants())
	{
		InputConstant newConstant;

		newConstant.m_BindingID = specConst.constant_id;

		const auto& constant = spirvComp.get_constant(specConst.id);
		const auto& constant_type = constant.constant_type;

		using BaseType = spirv_cross::SPIRType::BaseType;
		newConstant.m_Type = spirvComp.get_type(constant_type);

		newConstant.m_FullName = spirvComp.get_name(specConst.id);
		newConstant.ParseFullName();

		if (Enums::has_flag(newConstant.m_Decoration, InputConstant::Decoration::Parameter))
			AssertAR(, m_InputConstants.insert(std::make_pair(newConstant.m_FriendlyName, newConstant)), .second);
		else
			AssertAR(, m_InputConstants.insert(std::make_pair(newConstant.m_FullName, newConstant)), .second);
	}
}

void ShaderModuleData::InputConstant::ParseFullName()
{
	m_FriendlyName = m_FullName;
	m_Decoration = Decoration::None;

	if (StringTools::BeginsWith(m_FriendlyName, PREFIX_PARAMETER))
	{
		m_FriendlyName.erase(0, PREFIX_PARAMETER.size());
		m_Decoration = Enums::add_flag(m_Decoration, Decoration::Parameter);
	}
	else if (StringTools::BeginsWith(m_FriendlyName, PREFIX_INPUTINDEX))
	{
		m_FriendlyName.erase(0, PREFIX_INPUTINDEX.size());
		m_Decoration = Enums::add_flag(m_Decoration, Decoration::InputIndex);
	}
	else if (StringTools::BeginsWith(m_FriendlyName, PREFIX_INPUT))
	{
		m_FriendlyName.erase(0, PREFIX_INPUT.size());
		m_Decoration = Enums::add_flag(m_Decoration, Decoration::Input);
	}
	else if (StringTools::BeginsWith(m_FriendlyName, PREFIX_OUTPUTINDEX))
	{
		m_FriendlyName.erase(0, PREFIX_OUTPUTINDEX.size());
		m_Decoration = Enums::add_flag(m_Decoration, Decoration::OutputIndex);
	}
	else if (StringTools::BeginsWith(m_FriendlyName, PREFIX_OUTPUT))
	{
		m_FriendlyName.erase(0, PREFIX_OUTPUT.size());
		m_Decoration = Enums::add_flag(m_Decoration, Decoration::Output);
	}
	else if (StringTools::BeginsWith(m_FriendlyName, PREFIX_TEXMODE))
	{
		m_FriendlyName.erase(0, PREFIX_TEXMODE.size());
		m_Decoration = Enums::add_flag(m_Decoration, Decoration::TexMode);
	}
	else
		return;

	if (m_FriendlyName.size() > 1 && m_FriendlyName[0] == '_')
		m_FriendlyName.erase(0, 1);
}

void ShaderModuleData::BasicType::ParseFullName()
{
	m_FriendlyName = m_FullName;
	m_Decoration = Decoration::None;

	if (StringTools::BeginsWith(m_FriendlyName, PREFIX_PARAMETER))
	{
		m_FriendlyName.erase(0, PREFIX_PARAMETER.size());
		m_Decoration = Enums::add_flag(m_Decoration, Decoration::Parameter);
	}
	else
		return;

	if (StringTools::BeginsWith(m_FriendlyName, PREFIX_TEX1D))
	{
		m_FriendlyName.erase(0, PREFIX_TEX1D.size());
		m_Decoration = Enums::add_flag(m_Decoration, Decoration::Tex1D);
	}
	else if (StringTools::BeginsWith(m_FriendlyName, PREFIX_TEX2D))
	{
		m_FriendlyName.erase(0, PREFIX_TEX2D.size());
		m_Decoration = Enums::add_flag(m_Decoration, Decoration::Tex2D);
	}
	else if (StringTools::BeginsWith(m_FriendlyName, PREFIX_TEX3D))
	{
		m_FriendlyName.erase(0, PREFIX_TEX3D.size());
		m_Decoration = Enums::add_flag(m_Decoration, Decoration::Tex3D);
	}

	if (m_FriendlyName.size() > 1 && m_FriendlyName[0] == '_')
		m_FriendlyName.erase(0, 1);
}
