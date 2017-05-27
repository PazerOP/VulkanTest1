#include "stdafx.h"
#include "MaterialData.h"

#include "ContentPaths.h"
#include "ShaderGroup.h"
#include "ShaderGroupData.h"
#include "ShaderGroupManager.h"
#include "ShaderModuleData.h"

MaterialData::MaterialData(const std::filesystem::path& path) :
	MaterialData(name_from_path(ContentPaths::Materials(), path), JSONSerializer::FromFile(path).GetObject())
{
}

MaterialData::MaterialData(const std::string& name, const std::string& jsonStr) :
	MaterialData(name, JSONSerializer::FromString(jsonStr).GetObject())
{
}

MaterialData::MaterialData(const std::string& name, const JSONObject& json) :
	m_Name(name)
{
	const JSONObject& shaderGroup = json.GetObject("shaderGroup");

	const auto& shaderGroupName = shaderGroup.GetString("name");
	m_ShaderGroup = ShaderGroupManager::Instance().Find(shaderGroupName);

	if (!m_ShaderGroup)
		throw MissingShaderGroupException(m_Name, shaderGroupName);

	for (const auto& value : shaderGroup.GetObject("inputs"))
	{
		const auto& type = value.second.GetType();
		const auto& name = value.first;
		// Search through all parameters of all shaders in this group, looking for
		// a reference of this parameter. This is to just make sure you don't have
		// misspelled parameters/non-hooked-up parameters sitting around in materials.
		bool found = false;
		for (const auto& shaderDef : m_ShaderGroup->GetData().GetShaderModulesData())
		{
			if (!shaderDef)
				continue;

			if (shaderDef->HasInputFriendly(name))
			{
				switch (value.second.GetType())
				{
				case JSONDataType::Bool:
					m_Inputs.insert(std::make_pair(value.first, value.second.GetBool()));
					break;
				case JSONDataType::Number:
					m_Inputs.insert(std::make_pair(value.first, value.second.GetNumber()));
					break;
				case JSONDataType::String:
					m_Inputs.insert(std::make_pair(value.first, value.second.GetString()));
					break;

				default:
					Log::TagMsg(TAG, "Warning: Unknown shader group input \"{0}\" of type {1} in material \"{2}\"", value.first, type, m_Name);
				}

				found = true;
				break;
			}
		}

		if (!found)
			Log::TagMsg(TAG, "Warning: Unknown shader group parameter \"{0}\" referenced in material \"{1}\"", name, m_Name);
	}

	if (!m_ShaderGroup)
		throw std::runtime_error(StringTools::CSFormat("Encountered unknown shader group \"{0}\"", shaderGroupName));
}