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
		std::shared_ptr<const ShaderModuleData> m_ModuleData;

		// Mapping from material parameter names <---> shader parameter names
		// aka "friendly" names <---> "real" names
		std::map<std::string, std::string> m_ParameterMap;
	};

	const auto& GetName() const { return m_Name; }
	const auto& GetShaderDefinitions() const { return m_ShaderDefinitions; }

private:
	void LoadShaders(const JSONObject& root);

	std::filesystem::path m_Path;
	std::string m_Name;
	std::array<std::shared_ptr<const ShaderDefinition>, Enums::count<ShaderType>()> m_ShaderDefinitions;
};