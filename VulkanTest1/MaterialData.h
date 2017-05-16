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

	const std::string& GetName() const { return m_Name; }

	const std::shared_ptr<ShaderGroup>& GetShaderGroup() const { return m_ShaderGroup; }

	const std::map<std::string, std::string>& GetShaderGroupInputs() const { return m_ShaderGroupInputs; }

private:
	std::string m_Name;
	std::shared_ptr<ShaderGroup> m_ShaderGroup;
	std::map<std::string, std::string> m_ShaderGroupInputs;
};