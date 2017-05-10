#include "stdafx.h"
#include "GraphicsPipelineCreateInfo.h"

#include "LogicalDevice.h"
#include "Vulkan.h"

GraphicsPipelineCreateInfo::GraphicsPipelineCreateInfo() :
	GraphicsPipelineCreateInfo(Vulkan().GetLogicalDevice())
{
}

GraphicsPipelineCreateInfo::GraphicsPipelineCreateInfo(LogicalDevice& device)
{
	m_Device = &device;
}

const ShaderModule* GraphicsPipelineCreateInfo::GetShader(ShaderType type) const
{
	assert(validate_enum_value(type));
	return m_Shaders[underlying_value(type)].get();
}

void GraphicsPipelineCreateInfo::SetShader(const std::shared_ptr<ShaderModule>& shader)
{
	assert(&shader->GetDevice() == &GetDevice());
	m_Shaders[underlying_value(shader->GetType())] = shader;
}
