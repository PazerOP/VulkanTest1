#include "stdafx.h"
#include "ShaderModule.h"

#include "LogicalDevice.h"
#include "Vulkan.h"

#include <fstream>

std::unique_ptr<ShaderModule> ShaderModule::Create(const std::filesystem::path& path, ShaderType type)
{
	return Create(path, type, Vulkan().GetLogicalDevice());
}

std::unique_ptr<ShaderModule> ShaderModule::Create(const std::filesystem::path& path, ShaderType type, LogicalDevice& device)
{
	return std::unique_ptr<ShaderModule>(new ShaderModule(path, type, device));
}

ShaderModule::ShaderModule(const std::filesystem::path& path, ShaderType type, LogicalDevice& device)
{
	m_Device = &device;
	m_Type = type;

	std::ifstream stream(path.string(), std::ios::ate | std::ios::binary);

	const size_t fileSize = stream.tellg();
	std::vector<char> codeBytes(fileSize);
	stream.seekg(0);
	stream.read(codeBytes.data(), fileSize);

	vk::ShaderModuleCreateInfo createInfo;

	createInfo.setCodeSize(codeBytes.size());

	std::vector<uint32_t> alignedCode(codeBytes.size() / sizeof(uint32_t) + 1);
	memcpy(alignedCode.data(), codeBytes.data(), codeBytes.size());
	createInfo.setPCode(alignedCode.data());

	m_Shader = device->createShaderModuleUnique(createInfo);
}