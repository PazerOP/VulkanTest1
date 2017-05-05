#include "stdafx.h"
#include "GraphicsPipelineCreateInfo.h"

#include "Vulkan.h"

GraphicsPipelineCreateInfo::GraphicsPipelineCreateInfo() :
	GraphicsPipelineCreateInfo(Vulkan().GetLogicalDevice(), Vulkan().GetSwapchain())
{
}

GraphicsPipelineCreateInfo::GraphicsPipelineCreateInfo(const std::shared_ptr<LogicalDevice>& device, const std::shared_ptr<Swapchain>& swapchain)
{
	m_Device = device;
	m_Swapchain = swapchain;
}

const std::shared_ptr<const ShaderModule> GraphicsPipelineCreateInfo::GetShader(ShaderType type) const
{
	assert(validate_enum_value(type));
	return m_Shaders[underlying_value(type)];
}

void GraphicsPipelineCreateInfo::SetShader(ShaderType type, const std::shared_ptr<ShaderModule>& shader)
{
	assert(validate_enum_value(type));
	assert(shader->GetDevice() == GetDevice());
	m_Shaders[underlying_value(type)] = shader;
}
