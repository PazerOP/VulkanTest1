#include "stdafx.h"
#include "Material.h"

#include "DescriptorSet.h"
#include "LogicalDevice.h"
#include "MaterialData.h"
#include "SimpleVertex.h"

Material::Material(const std::shared_ptr<const MaterialData>& data, LogicalDevice& device) :
	m_Data(data), m_Device(device)
{
	auto createInfo = std::make_shared<GraphicsPipelineCreateInfo>();

	createInfo->m_ShaderGroup = data->GetShaderGroup();
	createInfo->m_DescriptorSetLayouts = device.GetBuiltinUniformBuffers().GetDescriptorSetLayouts();

	createInfo->m_VertexInputBindingDescription = SimpleVertex::GetBindingDescription();
	createInfo->m_VertexInputAttributeDescriptions = SimpleVertex::GetAttributeDescriptions();

	m_GraphicsPipeline.emplace(m_Device, createInfo);
}

void Material::Bind(const vk::CommandBuffer& cmdBuf) const
{
	const auto& pipeline = m_GraphicsPipeline.value();
	
	cmdBuf.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.GetPipeline());

	const auto& descriptorSets = GetDescriptorSets();
	cmdBuf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.GetPipelineLayout(), 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
}

std::vector<vk::DescriptorSet> Material::GetDescriptorSets() const
{
	std::vector<vk::DescriptorSet> retVal;

	for (const auto& setWrapper : GetPipeline().GetDescriptorSets())
		retVal.push_back(setWrapper->GetDescriptorSet());

	return retVal;
}
