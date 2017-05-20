#pragma once
#include "JSON.h"

#include <filesystem>

class ShaderGroup;

class MaterialData
{
public:
	MaterialData(const std::filesystem::path& path);
	MaterialData(const std::string& str);
	MaterialData(const JSONObject& json);

	const auto& GetName() const { return m_Name; }
	const auto& GetShaderGroup() const { return m_ShaderGroup; }
	const auto& GetShaderGroupInputs() const { return m_ShaderGroupInputs; }

	class MaterialDataLoadException : public BaseException<>
	{
	public:
		MaterialDataLoadException(const std::string& typeName, const std::string& materialName, const std::string& msg) :
			BaseException(typeName, StringTools::CSFormat("Failed to load material \"{0}\": {1}", materialName, msg))
		{
		}
	};
	class MissingShaderGroupException : public MaterialDataLoadException
	{
	public:
		MissingShaderGroupException(const std::string& materialName, const std::string& shaderGroupName) :
			MaterialDataLoadException("MissingShaderGroupException"s, materialName,
				StringTools::CSFormat("Failed to locate shader group \"{0}\"", shaderGroupName))
		{
		}
	};

private:
	std::string m_Name;
	std::shared_ptr<const ShaderGroup> m_ShaderGroup;
	std::map<std::string, std::string> m_ShaderGroupInputs;
};