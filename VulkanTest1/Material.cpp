#include "stdafx.h"
#include "Material.h"

#include "MaterialData.h"

Material::Material(const std::shared_ptr<const MaterialData>& data, LogicalDevice& device) :
	m_Data(data), m_Device(device)
{
	auto createInfo = std::make_shared<GraphicsPipelineCreateInfo>(device);

	createInfo->SetShaderGroup(data->GetShaderGroup());

	m_GraphicsPipeline.emplace(m_Device, createInfo);
}

void Material::Bind(const vk::CommandBuffer& cmdBuf) const
{
	const auto& pipeline = m_GraphicsPipeline.value();
	
	cmdBuf.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.GetPipeline());

	const auto& descriptorSets = pipeline.GetDescriptorSets();
	cmdBuf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.GetPipelineLayout(), 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
}
