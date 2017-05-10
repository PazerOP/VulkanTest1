#pragma once

#include <filesystem>

class LogicalDevice;

enum class ShaderType
{
	Vertex,
	TessellationControl,
	TessellationEvaluation,
	Gemoetry,
	Compute,
	Fragment,
	Pixel = Fragment,

	Count,
};
__forceinline bool validate_enum_value(ShaderType value)
{
	return underlying_value(value) >= 0 && underlying_value(value) < underlying_value(ShaderType::Count);
}

class ShaderModule
{
public:
	static std::unique_ptr<ShaderModule> Create(const std::filesystem::path& path, ShaderType type);
	static std::unique_ptr<ShaderModule> Create(const std::filesystem::path& path, ShaderType type, LogicalDevice& device);

	const std::filesystem::path& GetPath() const { return m_Path; }

	const LogicalDevice& GetDevice() const { assert(m_Device); return *m_Device; }
	LogicalDevice& GetDevice() { assert(m_Device); return *m_Device; }

	const vk::ShaderModule* operator->() const { return m_Shader.operator->(); }
	//vk::ShaderModule* operator->() { return &m_Shader; }

	const vk::ShaderModule Get() const { return m_Shader.get(); }
	vk::ShaderModule Get() { return m_Shader.get(); }

	ShaderType GetType() const { return m_Type; }

private:
	ShaderModule(const std::filesystem::path& path, ShaderType type, LogicalDevice& device);

	vk::UniqueShaderModule m_Shader;
	LogicalDevice* m_Device;
	ShaderType m_Type;
	std::filesystem::path m_Path;
};