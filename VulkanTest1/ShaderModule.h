#pragma once

#include <filesystem>

#if _MSC_VER == 1910
namespace std
{
	namespace filesystem = ::std::experimental::filesystem;
}
#endif

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
	ShaderModule(const std::filesystem::path& path);
	ShaderModule(const std::filesystem::path& path, const std::shared_ptr<LogicalDevice>& device);

	const std::filesystem::path& GetPath() const { return m_Path; }

	const std::shared_ptr<const LogicalDevice>& GetDevice() const { return m_Device; }
	const std::shared_ptr<LogicalDevice>& GetDevice() { return m_Device; }

	const std::shared_ptr<const vk::ShaderModule> GetShader() const { return m_Shader; }
	const std::shared_ptr<vk::ShaderModule>& GetShader() { return m_Shader; }

private:
	std::shared_ptr<vk::ShaderModule> m_Shader;
	std::shared_ptr<LogicalDevice> m_Device;

	std::filesystem::path m_Path;
};