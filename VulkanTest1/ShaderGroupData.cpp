#include "stdafx.h"
#include "ShaderGroupData.h"

#include "BuiltinUniformBuffers.h"
#include "ContentPaths.h"
#include "ShaderModuleData.h"
#include "ShaderModuleDataManager.h"

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
	LoadShaders(json);
}

void ShaderGroupData::LoadShaders(const JSONObject& root)
{
	for (const JSONValue& value : root.GetArray("shaders"))
	{
		const JSONObject& shaderObj = value.GetObject();

		std::shared_ptr<ShaderDefinition> def = std::make_shared<ShaderDefinition>();
		def->m_ModuleData = ShaderModuleDataManager::Instance().Find(shaderObj.GetString("file"));

		const JSONObject* parametersArray = shaderObj.TryGetObject("parameters");
		if (parametersArray)
		{
			for (const auto& value : *parametersArray)
				def->m_ParameterMap.insert(std::make_pair(value.first, value.second.GetString()));
		}

		assert(!m_ShaderDefinitions[Enums::value(def->m_ModuleData->GetType())]);
		m_ShaderDefinitions[Enums::value(def->m_ModuleData->GetType())] = def;
	}
}