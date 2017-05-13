#include "stdafx.h"
#include "MaterialData.h"

#include "ShaderGroupManager.h"

MaterialData::MaterialData(const std::filesystem::path& path) :
	MaterialData(JSONSerializer::FromFile(path).GetObject())
{
}

MaterialData::MaterialData(const std::string& str) :
	MaterialData(JSONSerializer::FromString(str).GetObject())
{
}

MaterialData::MaterialData(const JSONObject& json)
{
	m_Name = json.find("name")->second.GetString();

	const auto& shaderGroupName = json.find("shaderGroup")->second.GetString();
	m_ShaderGroup = ShaderGroupManager::Instance().Find(shaderGroupName);

	if (!m_ShaderGroup)
		throw std::runtime_error(StringTools::CSFormat("Encountered unknown shader group \"{0}\"", shaderGroupName));
}
