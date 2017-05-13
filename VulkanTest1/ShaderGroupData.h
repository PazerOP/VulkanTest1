#pragma once
#include "JSON.h"
#include "ShaderParameterType.h"
#include "ShaderType.h"

#include <filesystem>

class ShaderGroupData
{
	using ShaderParameterList = std::map<std::string, std::vector<ShaderParameterType>>;
public:
	ShaderGroupData(const std::filesystem::path& path);
	ShaderGroupData(const std::string& str);
	ShaderGroupData(const JSONObject& json);

	class ParseException : public std::runtime_error
	{
	private:
		ParseException(const std::string& str) : std::runtime_error(str) { Log::Msg<LogType::Exception>(str); }
		friend class ShaderGroupData;
	};

	struct ShaderBinding
	{
		uint32_t m_BindingIndex;
		std::string m_ParameterName;
	};

	struct ShaderDefinition
	{
		ShaderType m_Type;
		std::filesystem::path m_Path;
		std::vector<ShaderBinding> m_Bindings;
	};

	const auto& GetName() const { return m_Name; }
	const auto& GetParameters() const { return m_Parameters; }
	const auto& GetShaderDefinitions() const { return m_Shaders; }

private:
	std::filesystem::path m_Path;

	bool IsValidParameterName(const std::string& paramName) const;

	std::string m_Name;
	ShaderParameterList m_Parameters;
	std::vector<ShaderDefinition> m_Shaders;
};