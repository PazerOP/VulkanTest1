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

		const auto& file = shaderObj.GetString("file");
		const auto moduleData = ShaderModuleDataManager::Instance().Find(file);
		assert(moduleData);

		const auto index = Enums::value(moduleData->GetType());
		assert(!m_ShaderModulesData[index]);
		m_ShaderModulesData[index] = moduleData;
	}
}