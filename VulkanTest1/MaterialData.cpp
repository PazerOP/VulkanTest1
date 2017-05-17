#include "stdafx.h"
#include "MaterialData.h"

#include "ShaderGroup.h"
#include "ShaderGroupData.h"
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

	const auto& shaderGroupParameters = m_ShaderGroup->GetData()->GetParameters();

	for (const auto& value : shaderGroup.find("inputs")->second.GetObject())
	{
		const auto& type = value.second.GetType();
		const auto& name = value.first;
		if (type == JSONDataType::String)
		{
			if (shaderGroupParameters.find(name) != shaderGroupParameters.end())
				m_ShaderGroupInputs.insert(std::make_pair(value.first, value.second.GetString()));
			else
				Log::Msg("Unknown shader group parameter \"{0}\" referenced in material \"{1}\"", name, m_Name);
		}
		else
			Log::Msg("Unknown shader group input \"{0}\" of type {1} in material \"{2}\"", value.first, type, m_Name);
	}

	if (!m_ShaderGroup)
		throw std::runtime_error(StringTools::CSFormat("Encountered unknown shader group \"{0}\"", shaderGroupName));
}
