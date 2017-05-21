#include "stdafx.h"
#include "ShaderModuleData.h"

#include <spirv_cross.hpp>

#include <fstream>
#include <map>

ShaderModuleData::ShaderModuleData(const std::filesystem::path& path)
{
	std::ifstream stream(path.string(), std::ios::ate | std::ios::binary);

	if (!stream.is_open())
		throw std::runtime_error(StringTools::CSFormat("Failed to open file \"{0}\"", path));

	const size_t fileSize = stream.tellg();
	std::vector<uint32_t> codeBytes((fileSize + sizeof(uint32_t) - 1) / sizeof(uint32_t));
	codeBytes.back() = 0;
	stream.seekg(0);
	stream.read((char*)codeBytes.data(), fileSize);

	spirv_cross::Compiler compiler(codeBytes);
	LoadShaderType(compiler);
	LoadSpecConstants(compiler);
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

void ShaderModuleData::LoadSpecConstants(const spirv_cross::Compiler& spirvComp)
{
	for (const auto& specConst : spirvComp.get_specialization_constants())
	{
		m_SpecConstants.emplace_back();
		SpecConstant& newConstant = m_SpecConstants.back();

		newConstant.m_BindingID = specConst.constant_id;
		newConstant.m_Name = spirvComp.get_name(specConst.id);

		const auto& constant = spirvComp.get_constant(specConst.id);
		const auto& constant_type = constant.constant_type;

		newConstant.m_Type = spirvComp.get_type(constant_type);
	}
}