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
	m_Name = json.find("name")->second.GetString();

	// Load parameters
	for (const JSONValue& value : json.find("parameters")->second.GetArray())
	{
		const JSONObject& parameterObj = value.GetObject();
		std::vector<ShaderParameterType> parameterTypes;

		const std::string& typeName = parameterObj.find("type")->second.GetString();
		std::optional<ShaderParameterType> paramType;
		if (!typeName.compare("texture"))
			paramType = ShaderParameterType::Texture;
		else
			throw ParseException(StringTools::CSFormat(__FUNCTION__ ": Unexpected shader parameter type \"{0}\"", typeName));

		m_Parameters.insert(std::make_pair(parameterObj.find("name")->second.GetString(), paramType.value()));
	}

	// Load shaders
	for (const JSONValue& value : json.find("shaders")->second.GetArray())
	{
		const JSONObject& shaderObj = value.GetObject();

		std::shared_ptr<ShaderDefinition> def = std::make_shared<ShaderDefinition>();
		def->m_Path = std::filesystem::current_path().append(std::filesystem::path(shaderObj.find("filename")->second.GetString()));

		const std::string& typeName = shaderObj.find("type")->second.GetString();
		if (!typeName.compare("vertex"))
			def->m_Type = ShaderType::Vertex;
		else if (!typeName.compare("tessellationControl"))
			def->m_Type = ShaderType::TessellationControl;
		else if (!typeName.compare("tessellationEvaluation"))
			def->m_Type = ShaderType::TessellationEvaluation;
		else if (!typeName.compare("geometry"))
			def->m_Type = ShaderType::Gemoetry;
		else if (!typeName.compare("compute"))
			def->m_Type = ShaderType::Compute;
		else if (!typeName.compare("fragment"))
			def->m_Type = ShaderType::Fragment;
		else
			throw ParseException(StringTools::CSFormat(__FUNCTION__ ": Unexpected shader type \"{0}\"", typeName));

		for (const JSONValue& input : shaderObj.find("inputs")->second.GetArray())
		{
			const JSONObject& inputObj = input.GetObject();
			ShaderBinding binding;

			binding.m_BindingIndex = (uint32_t)inputObj.find("binding")->second.GetNumber();
			binding.m_ParameterName = inputObj.find("parameter")->second.GetString();

			if (binding.m_BindingIndex <= Enums::max<BuiltinUniformBuffers::FrameViewBindings>())
				throw ParseException(StringTools::CSFormat(__FUNCTION__ ": Attempted to bind parameter \"{0}\" to reserved binding index {1} on {2} shader in group {3}! All indices below {4} are reserved for builtin uniform buffers.", binding.m_ParameterName, binding.m_BindingIndex, typeName, m_Name, Enums::max<BuiltinUniformBuffers::FrameViewBindings>()));

			if (!IsValidParameterName(binding.m_ParameterName))
				throw ParseException(StringTools::CSFormat(__FUNCTION__ ": Attempted to bind invalid parameter name \"{0}\" @ binding {1} on {2} shader in group {3}.", binding.m_ParameterName, binding.m_BindingIndex, typeName, m_Name));

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
