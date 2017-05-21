#include "stdafx.h"
#include "ShaderGroupData.h"

#include "BuiltinUniformBuffers.h"
#include "ContentPaths.h"

#include <fstream>

ShaderGroupData::ShaderGroupData(const std::filesystem::path& path) :
	ShaderGroupData(name_from_path(ContentPaths::Shaders(), path), JSONSerializer::FromFile(path).GetObject())
{
}

ShaderGroupData::ShaderGroupData(const std::string& name, const std::string& str) :
	ShaderGroupData(name, JSONSerializer::FromString(str).GetObject())
{
}

ShaderGroupData::ShaderGroupData(const std::string& name, const JSONObject& json) :
	m_Name(name)
{
	LoadParameters(json);
	LoadSpecializationConstants(json);
	LoadShaders(json);
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

void ShaderGroupData::LoadParameters(const JSONObject& root)
{
	const JSONArray* parametersArray = root.TryGetArray("parameters");
	if (!parametersArray)
		return;

	static const std::map<std::string, ShaderParameterType> s_ParamTypes =
	{
		{ "texture"s, ShaderParameterType::Texture },
		{ "float"s, ShaderParameterType::Float },
		{ "int"s, ShaderParameterType::Int },
		{ "bool"s, ShaderParameterType::Bool },
	};

	for (const JSONValue& value : *parametersArray)
	{
		const JSONObject& parameterObj = value.GetObject();
		std::vector<ShaderParameterType> parameterTypes;

		const auto& paramName = parameterObj.GetString("name");
		const std::string& paramTypeName = parameterObj.GetString("type");

		ShaderParameterType paramType;
		try
		{
			paramType = s_ParamTypes.at(paramTypeName);
		}
		catch (std::out_of_range)
		{
			throw ParseException(StringTools::CSFormat(__FUNCTION__ ": Unexpected shader parameter type \"{0}\" for parameter \"{1}\" in shader \"{2}\". Valid parameter types are \"texture\", \"float\", \"int\", and \"bool\".", paramTypeName, paramType, m_Name));
		}

		m_Parameters.insert(std::make_pair(paramName, paramType));
	}
}

void ShaderGroupData::LoadSpecializationConstants(const JSONObject& root)
{
	const JSONArray* parametersArray = root.TryGetArray("specializationConstants");
	if (!parametersArray)
		return;

	static const std::map<std::string, ShaderParameterType> s_ParamTypes =
	{
		{ "float"s, ShaderParameterType::Float },
		{ "int"s, ShaderParameterType::Int },
		{ "bool"s, ShaderParameterType::Bool },
	};

	for (const JSONValue& value : *parametersArray)
	{
		const JSONObject& parameterObj = value.GetObject();
		std::vector<ShaderParameterType> parameterTypes;

		const auto& paramName = parameterObj.GetString("name");
		const std::string& paramTypeName = parameterObj.GetString("type");

		ShaderParameterType paramType;
		try
		{
			paramType = s_ParamTypes.at(paramTypeName);
		}
		catch (std::out_of_range)
		{
			throw ParseException(StringTools::CSFormat(__FUNCTION__ ": Unexpected shader specialization constant type \"{0}\" for specialization constant \"{1}\" in shader \"{2}\". Valid specialization constant types are \"float\", \"int\", and \"bool\".", paramTypeName, paramType, m_Name));
		}

		m_Parameters.insert(std::make_pair(paramName, paramType));
	}
}

void ShaderGroupData::LoadShaders(const JSONObject& root)
{
	static const std::map<std::string, ShaderType> s_ShaderTypes =
	{
		{ "vertex"s, ShaderType::Vertex },
		{ "tessellationControl"s, ShaderType::TessellationControl },
		{ "tessellationEvaluation"s, ShaderType::TessellationEvaluation },
		{ "geometry"s, ShaderType::Geometry },
		{ "compute"s, ShaderType::Compute },
		{ "fragment"s, ShaderType::Fragment },
	};

	for (const JSONValue& value : root.GetArray("shaders"))
	{
		const JSONObject& shaderObj = value.GetObject();

		std::shared_ptr<ShaderDefinition> def = std::make_shared<ShaderDefinition>();
		def->m_Path = std::filesystem::current_path().append(std::filesystem::path(shaderObj.GetString("filename")));

		const std::string& typeName = shaderObj.GetString("type");

		try
		{
			def->m_Type = s_ShaderTypes.at(typeName);
		}
		catch (std::out_of_range)
		{
			throw ParseException(StringTools::CSFormat(__FUNCTION__ ": Unexpected shader type \"{0}\" in shader \"{1}\". Valid shader types are \"vertex\", \"tessellationControl\", \"tessellationEvaluation\", \"geometry\", \"compute\", and \"fragment\".", typeName, m_Name));
		}

		const JSONArray* inputsArray = shaderObj.TryGetArray("inputs");
		if (inputsArray)
		{
			for (const JSONValue& input : *inputsArray)
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
		}

		m_Shaders.push_back(def);
	}
}

bool ShaderGroupData::IsValidParameterName(const std::string& paramName) const
{
	if (m_Parameters.find(paramName) != m_Parameters.end())
		return true;

	return false;
}