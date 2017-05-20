#pragma once
#include "BaseException.h"
#include "JSON.h"
#include "ShaderParameterType.h"
#include "ShaderType.h"

#include <filesystem>

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

	struct ShaderBinding
	{
		std::optional<uint32_t> m_BindingIndex;
		std::string m_ParameterName;
	};

	struct ShaderDefinition
	{
		std::optional<ShaderType> m_Type;
		std::filesystem::path m_Path;
		std::vector<ShaderBinding> m_Bindings;
	};

	const auto& GetName() const { return m_Name; }
	const auto& GetParameters() const { return m_Parameters; }
	const auto& GetShaderDefinitions() const { return m_Shaders; }

	struct ParameterDependency
	{
		uint32_t m_BindingIndex;
		std::shared_ptr<const ShaderDefinition> m_Definition;
	};
	std::vector<ParameterDependency> FindByParameterDependency(const std::string& paramName) const;

private:
	void LoadParameters(const JSONObject& root);
	void LoadSpecializationConstants(const JSONObject& root);
	void LoadShaders(const JSONObject& root);

	std::filesystem::path m_Path;

	bool IsValidParameterName(const std::string& paramName) const;

	std::string m_Name;
	std::map<std::string, ShaderParameterType> m_Parameters;
	std::map<std::string, ShaderParameterType> m_SpecializationConstants;
	std::vector<std::shared_ptr<ShaderDefinition>> m_Shaders;
};