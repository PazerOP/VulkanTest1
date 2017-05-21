#pragma once
#include "BaseException.h"
#include "JSON.h"
#include "ShaderParameterType.h"
#include "ShaderType.h"

#include <filesystem>

class ShaderModuleData;

class ShaderGroupData
{
public:
	ShaderGroupData(const std::filesystem::path& path);
	ShaderGroupData(const std::string& name, const std::string& str);
	ShaderGroupData(const std::string& name, const JSONObject& json);

	class ParseException : public BaseException<>
	{
	private:
		ParseException(const std::string& str) : BaseException("ShaderGroupData::ParseException"s, str) { }
		friend class ShaderGroupData;
	};

	struct ShaderDefinition
	{
		std::shared_ptr<ShaderModuleData> m_ShaderModule;
		std::map<std::string, std::string> m_Parameters;
	};

	const auto& GetName() const { return m_Name; }
	const auto& GetShaderDefinitions() const { return m_Shaders; }

	struct ParameterDependency
	{
		uint32_t m_BindingIndex;
		std::shared_ptr<const ShaderDefinition> m_Definition;
	};
	std::vector<ParameterDependency> FindByParameterDependency(const std::string& paramName) const;

private:
	void LoadShaders(const JSONObject& root);

	std::filesystem::path m_Path;
	std::string m_Name;
	std::vector<std::shared_ptr<ShaderDefinition>> m_Shaders;
};