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

	const JSONObject& shaderGroup = json.find("shaderGroup")->second.GetObject();

	const auto& shaderGroupName = shaderGroup.find("name")->second.GetString();
	m_ShaderGroup = ShaderGroupManager::Instance().Find(shaderGroupName);

	for (const auto& value : shaderGroup)
	{
		const auto type = value.second.GetType();
		if (type == JSONDataType::String)
			m_ShaderGroupInputs.insert(std::make_pair(value.first, value.second.GetString()));
		else
			Log::Msg("Unknown shader group input \"{0}\" of type {1} in material \"{2}\"", value.first, type, m_Name);
	}

	if (!m_ShaderGroup)
		throw std::runtime_error(StringTools::CSFormat("Encountered unknown shader group \"{0}\"", shaderGroupName));
}
