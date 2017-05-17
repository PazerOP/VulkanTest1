#include "stdafx.h"
#include "Material.h"

#include "DescriptorSet.h"
#include "DescriptorSetCreateInfo.h"
#include "DescriptorSetLayout.h"
#include "DescriptorSetLayoutCreateInfo.h"
#include "LogicalDevice.h"
#include "MaterialData.h"
#include "ShaderGroup.h"
#include "ShaderGroupData.h"
#include "SimpleVertex.h"
#include "TextureManager.h"

Material::Material(const std::shared_ptr<const MaterialData>& data, LogicalDevice& device) :
	m_Data(data), m_Device(device)
{
	InitTextures();
	InitGraphicsPipeline();
	InitDescriptorSet();
}

void Material::Bind(const vk::CommandBuffer& cmdBuf) const
{
	const auto& pipeline = m_GraphicsPipeline.value();

	cmdBuf.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.GetPipeline());

	const auto& descriptorSets = GetDescriptorSets();
	cmdBuf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.GetPipelineLayout(), 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
}

void Material::InitTextures()
{
	const auto& paramDefs = m_Data->GetShaderGroup()->GetData()->GetParameters();

	for (const auto& param : m_Data->GetShaderGroupInputs())
	{
		const auto& found = paramDefs.find(param.first);
		if (found == paramDefs.end())
		{
			Log::Msg("Shader param \"{0}\" (value \"{1}\") encountered in material \"{2}\" was not defined in shader \"{3}\"",
					 param.first, param.second, m_Data->GetName(), m_Data->GetShaderGroup()->GetData()->GetName());
			continue;
		}

		switch (found->second)
		{
		case ShaderParameterType::Texture:
			LoadTexture(param.first, param.second);
			break;
		}
	}
}

void Material::InitGraphicsPipeline()
{
	auto createInfo = std::make_shared<GraphicsPipelineCreateInfo>();

	createInfo->m_ShaderGroup = m_Data->GetShaderGroup();
	createInfo->m_DescriptorSetLayouts = m_Device.GetBuiltinUniformBuffers().GetDescriptorSetLayouts();
	createInfo->m_DescriptorSetLayouts.push_back(m_DescriptorSetLayout);

	createInfo->m_VertexInputBindingDescription = SimpleVertex::GetBindingDescription();
	createInfo->m_VertexInputAttributeDescriptions = SimpleVertex::GetAttributeDescriptions();

	m_GraphicsPipeline.emplace(m_Device, createInfo);
}

void Material::InitDescriptorSet()
{
	auto createInfo = std::make_shared<DescriptorSetCreateInfo>();

	//createInfo->m_Data = nullptr;
	createInfo->m_Layout = m_DescriptorSetLayout;

	m_DescriptorSet = std::make_shared<DescriptorSet>(m_Device, createInfo);
}

void Material::LoadTexture(const std::string& paramName, const std::string& textureName)
{
	const auto& texture = TextureManager::Instance().Find(textureName);
	assert(texture);
	if (!texture)
		return;

	m_Textures.insert(std::make_pair(paramName, texture));
}

void Material::InitDescriptorSetLayout()
{
	if (!m_Textures.size())
		return;

	auto newCreateInfo = std::make_shared<DescriptorSetLayoutCreateInfo>();

	for (const auto& texture : m_Textures)
	{
		const auto& dependentStages = m_Data->GetShaderGroup()->GetData()->FindByParameterDependency(texture.first);
		assert(!dependentStages.empty());

		std::map<uint32_t, vk::ShaderStageFlags> stages;

		for (const auto& dependentStage : dependentStages)
		{
			const auto type = Enums::convert<vk::ShaderStageFlagBits>(dependentStage.m_Definition->m_Type);
			auto found = stages.find(dependentStage.m_BindingIndex);
			if (found != stages.end())
				found->second |= Enums::convert<vk::ShaderStageFlagBits>(dependentStage.m_Definition->m_Type);
			else
				stages.insert(std::make_pair(dependentStage.m_BindingIndex, type));
		}

		for (const auto& groupedStage : stages)
		{
			vk::DescriptorSetLayoutBinding binding;

			binding.setStageFlags(groupedStage.second);
			binding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
			binding.setBinding(groupedStage.first);
			binding.setDescriptorCount(1);

			newCreateInfo->m_Bindings.push_back(binding);
		}
	}

	m_DescriptorSetLayout = std::make_shared<DescriptorSetLayout>(m_Device, newCreateInfo);
}

std::vector<vk::DescriptorSet> Material::GetDescriptorSets() const
{
	std::vector<vk::DescriptorSet> retVal;

	for (const auto& setWrapper : GetPipeline().GetDescriptorSets())
		retVal.push_back(setWrapper->GetDescriptorSet());

	return retVal;
}
