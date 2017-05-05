#include "stdafx.h"
#include "ShaderModule.h"

#include "LogicalDevice.h"
#include "Vulkan.h"

#include <fstream>

ShaderModule::ShaderModule(const std::filesystem::path & path) :
	ShaderModule(path, Vulkan().GetLogicalDevice())
{
}

ShaderModule::ShaderModule(const std::filesystem::path& path, const std::shared_ptr<LogicalDevice>& device)
{
	m_Device = device;

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

	m_Shader = std::shared_ptr<vk::ShaderModule>(
		new vk::ShaderModule(m_Device->GetDevice().createShaderModule(createInfo)),
		[device](vk::ShaderModule* sm) { device->GetDevice().destroyShaderModule(*sm); delete sm; }
		);
}