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
	m_Name = json.GetString("name");

	const JSONObject& shaderGroup = json.GetObject("shaderGroup");

	const auto& shaderGroupName = shaderGroup.GetString("name");
	m_ShaderGroup = ShaderGroupManager::Instance().Find(shaderGroupName);

	if (!m_ShaderGroup)
		throw MissingShaderGroupException(m_Name, shaderGroupName);

	for (const auto& value : shaderGroup.GetObject("inputs"))
	{
		const auto& type = value.second.GetType();
		const auto& name = value.first;
		if (type == JSONDataType::String)
		{
			// Search through all parameters of all shaders in this group, looking for
			// a reference of this parameter. This is to just make sure you don't have
			// misspelled parameters/non-hooked-up parameters sitting around in materials.
			bool found = false;
			for (const auto& shaderDef : m_ShaderGroup->GetData().GetShaderDefinitions())
			{
				if (shaderDef->m_ParameterMap.find(name) != shaderDef->m_ParameterMap.end())
				{
					m_Inputs.insert(std::make_pair(value.first, value.second.GetString()));
					found = true;
					break;
				}
			}

			if (!found)
				Log::Msg("Unknown shader group parameter \"{0}\" referenced in material \"{1}\"", name, m_Name);
		}
		else
			Log::Msg("Unknown shader group input \"{0}\" of type {1} in material \"{2}\"", value.first, type, m_Name);
	}

	if (!m_ShaderGroup)
		throw std::runtime_error(StringTools::CSFormat("Encountered unknown shader group \"{0}\"", shaderGroupName));
}