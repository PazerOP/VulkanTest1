#pragma once
#include "ShaderType.h"

#include <filesystem>

class LogicalDevice;

class ShaderModule
{
public:
	ShaderModule(const std::filesystem::path& path, ShaderType type, LogicalDevice& device);

	const std::filesystem::path& GetPath() const { return m_Path; }

	const LogicalDevice& GetDevice() const { assert(m_Device); return *m_Device; }
	LogicalDevice& GetDevice() { assert(m_Device); return *m_Device; }

	const vk::ShaderModule* operator->() const { return m_Shader.operator->(); }
	//vk::ShaderModule* operator->() { return &m_Shader; }

	const vk::ShaderModule Get() const { return m_Shader.get(); }
	vk::ShaderModule Get() { return m_Shader.get(); }

	ShaderType GetType() const { return m_Type; }

private:

	vk::UniqueShaderModule m_Shader;
	LogicalDevice* m_Device;
	ShaderType m_Type;
	std::filesystem::path m_Path;
};