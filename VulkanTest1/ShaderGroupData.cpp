#include "stdafx.h"
#include "ShaderGroupData.h"

#include "BuiltinUniformBuffers.h"

ShaderGroupData::ShaderGroupData(const std::filesystem::path& path) :
	ShaderGroupData(JSONSerializer::FromFile(path).GetObject())
{
}

ShaderGroupData::ShaderGroupData(const std::string& str) :
	ShaderGroupData(JSONSerializer::FromString(str).GetObject())
{
}

ShaderGroupData::ShaderGroupData(const JSONObject& json)
{
	// Load name
	m_Name = json.GetString("name");

	// Load parameters
	for (const JSONValue& value : json.GetArray("parameters"))
	{
		const JSONObject& parameterObj = value.GetObject();
		std::vector<ShaderParameterType> parameterTypes;

		const std::string& typeName = parameterObj.GetString("type");
		std::optional<ShaderParameterType> paramType;
		if (!typeName.compare("texture"))
			paramType = ShaderParameterType::Texture;
		else
			throw ParseException(StringTools::CSFormat(__FUNCTION__ ": Unexpected shader parameter type \"{0}\"", typeName));

		m_Parameters.insert(std::make_pair(parameterObj.GetString("name"), paramType.value()));
	}

	// Load shaders
	for (const JSONValue& value : json.GetArray("shaders"))
	{
		const JSONObject& shaderObj = value.GetObject();

		std::shared_ptr<ShaderDefinition> def = std::make_shared<ShaderDefinition>();
		def->m_Path = std::filesystem::current_path().append(std::filesystem::path(shaderObj.GetString("filename")));

		const std::string& typeName = shaderObj.GetString("type");
		if (typeName == "vertex"sv)
			def->m_Type = ShaderType::Vertex;
		else if (typeName == "tessellationControl"sv)
			def->m_Type = ShaderType::TessellationControl;
		else if (typeName == "tessellationEvaluation"sv)
			def->m_Type = ShaderType::TessellationEvaluation;
		else if (typeName == "geometry"sv)
			def->m_Type = ShaderType::Gemoetry;
		else if (typeName == "compute"sv)
			def->m_Type = ShaderType::Compute;
		else if (typeName == "fragment"sv)
			def->m_Type = ShaderType::Fragment;
		else
			throw ParseException(StringTools::CSFormat(__FUNCTION__ ": Unexpected shader type \"{0}\"", typeName));

		for (const JSONValue& input : shaderObj.GetArray("inputs"))
		{
			const JSONObject& inputObj = input.GetObject();
			ShaderBinding binding;

			binding.m_ParameterName = inputObj.GetString("parameter");

			if (!IsValidParameterName(binding.m_ParameterName))
				throw ParseException(StringTools::CSFormat(__FUNCTION__ ": Attempted to bind invalid parameter name \"{0}\" on {1} shader in group {2}.", binding.m_ParameterName, typeName, m_Name));

			const bool isTexture = m_Parameters.at(binding.m_ParameterName) == ShaderParameterType::Texture;

			binding.m_BindingIndex = (uint32_t)inputObj.GetNumber("binding");

			def->m_Bindings.push_back(binding);
		}

		m_Shaders.push_back(def);
	}
}

std::vector<ShaderGroupData::ParameterDependency> ShaderGroupData::FindByParameterDependency(const std::string& paramName) const
{
	std::vector<ParameterDependency> retVal;

	for (const auto& shaderDef : m_Shaders)
	{
		for (const auto& binding : shaderDef->m_Bindings)
		{
			if (!binding.m_ParameterName.compare(paramName))
			{
				retVal.emplace_back();
				ParameterDependency& newDep = retVal.back();

				newDep.m_BindingIndex = binding.m_BindingIndex.value();
				newDep.m_Definition = shaderDef;

				break;
			}
		}
	}

	return retVal;
}

bool ShaderGroupData::IsValidParameterName(const std::string& paramName) const
{
	if (m_Parameters.find(paramName) != m_Parameters.end())
		return true;

	return false;
}