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
#include "ShaderModuleData.h"
#include "SimpleVertex.h"
#include "Texture.h"
#include "TextureManager.h"
#include "shaders/interop.h"

#include <set>

Material::Material(const std::shared_ptr<const MaterialData>& data, LogicalDevice& device) :
	m_Data(data), m_Device(device)
{
	InitResources();
	InitDescriptorSetLayout();
	InitGraphicsPipeline();
	InitDescriptorSet();
}

void Material::Bind(const vk::CommandBuffer& cmdBuf) const
{
	const auto& pipeline = m_GraphicsPipeline.value();

	cmdBuf.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.GetPipeline());

	for (const auto& descriptorSetGroup : GetDescriptorSets())
	{
		cmdBuf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.GetPipelineLayout(), descriptorSetGroup.first, descriptorSetGroup.second.size(), descriptorSetGroup.second.data(), 0, nullptr);
	}
}

void Material::InitResources()
{
	// For each material input parameter
	for (const auto& inputParam : m_Data->GetInputs())
	{
		// For each shader definition in the shader group
		bool used = false;
		for (const auto& shaderDef : GetData().GetShaderGroup().GetData().GetShaderDefinitions())
		{
			const auto& foundRealParamName = shaderDef->m_ParameterMap.find(inputParam.first);
			if (foundRealParamName == shaderDef->m_ParameterMap.end())
				continue;
			else
				used = true;

			const auto& moduleData = shaderDef->m_ModuleData;
			const auto& inputParamData = moduleData->GetInputParams().at(foundRealParamName->second);

			switch (inputParamData.m_Type.basetype)
			{
			case spirv_cross::SPIRType::BaseType::Sampler:
				m_Resources_[moduleData->GetType()].m_Textures.insert(std::make_pair(
					inputParamData.m_BindingID, TextureManager::Instance().Find(inputParam.second)));
				break;
			default:
				assert(!"Not hooked up for this type!");
				break;
			}
		}

		if (!used)
		{
			Log::Msg("Shader param \"{0}\" (value \"{1}\") encountered in material \"{2}\" was not defined in shader group \"{3}\"",
				inputParam.first, inputParam.second, m_Data->GetName(), m_Data->GetShaderGroup().GetData().GetName());
			continue;
		}
	}
}

void Material::InitGraphicsPipeline()
{
	auto createInfo = std::make_shared<GraphicsPipelineCreateInfo>();

	createInfo->m_ShaderGroup = m_Data->GetShaderGroupPtr();
	createInfo->m_DescriptorSetLayouts = m_Device.GetBuiltinUniformBuffers().GetDescriptorSetLayoutsUint();
	createInfo->m_DescriptorSetLayouts.insert(std::make_pair(Enums::value(BuiltinUniformBuffers::Set::Material), m_DescriptorSetLayout));

	createInfo->m_VertexInputBindingDescription = SimpleVertex::GetBindingDescription();
	createInfo->m_VertexInputAttributeDescriptions = SimpleVertex::GetAttributeDescriptions();

	createInfo->m_Specializations = SetupSpecializations();

	m_GraphicsPipeline.emplace(m_Device, createInfo);
}

void Material::InitDescriptorSet()
{
	auto createInfo = std::make_shared<DescriptorSetCreateInfo>();
	createInfo->m_Layout = m_DescriptorSetLayout;

	for (const auto& resource : m_Resources_)
	{
		for (const auto& texture : resource.second.m_Textures)
		{
			createInfo->m_Data.emplace_back();
			auto& binding = createInfo->m_Data.back();
			binding.m_Stages = Enums::convert<vk::ShaderStageFlags>(resource.first);
			binding.m_BindingIndex = texture.first;
			binding.m_Data = texture.second;
			binding.m_DebugName = __FUNCSIG__;
		}
	}

	m_DescriptorSet = std::make_shared<DescriptorSet>(m_Device, createInfo);
}

void Material::InitDescriptorSetLayout()
{
	auto newCreateInfo = std::make_shared<DescriptorSetLayoutCreateInfo>();
	newCreateInfo->m_DebugName = __FUNCSIG__;

	for (const auto& shaderDef : GetData().GetShaderGroup().GetData().GetShaderDefinitions())
	{
		const auto stageFlagBits = Enums::convert<vk::ShaderStageFlagBits>(shaderDef->m_ModuleData->GetType());

		// See what parameters this stage has
		for (const auto& param : shaderDef->m_ParameterMap)
		{
			const auto& realParam = shaderDef->m_ModuleData->GetInputParams().at(param.second);

			newCreateInfo->m_Bindings.emplace_back();

		}
	}

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
			binding.setDescriptorCount(1);
			binding.setBinding(groupedStage.first);

			newCreateInfo->m_Bindings.push_back(binding);
		}
	}

	m_DescriptorSetLayout = std::make_shared<DescriptorSetLayout>(m_Device, newCreateInfo);
}

// Breaks up all the descriptor sets that need to be bound for this material into
// contiguous groups so we can make fewer calls to vkBindDescriptorSets. The key is
// the start index of each set.
std::map<uint32_t, std::vector<vk::DescriptorSet>> Material::GetDescriptorSets() const
{
	auto ordered = m_Device.GetBuiltinUniformBuffers().GetDescriptorSets();
	ordered.insert(std::make_pair(BuiltinUniformBuffers::Set::Material, m_DescriptorSet));

	std::map<uint32_t, std::vector<vk::DescriptorSet>> retVal;
	auto start = ordered.begin();
	auto previous = ordered.begin();
	for (auto it = std::next(ordered.begin()); it != ordered.end(); it++)
	{
		if (Enums::value(it->first) > (Enums::value(previous->first) + 1))
		{
			// Break ourselves off
			std::vector<vk::DescriptorSet> descriptorSets;
			for (auto brokenOffIt = start; brokenOffIt != it; brokenOffIt++)
				descriptorSets.push_back(brokenOffIt->second->GetDescriptorSet());

			retVal.insert(std::make_pair(Enums::value(start->first), std::move(descriptorSets)));

			start = it;
		}

		previous = it;
	}

	std::vector<vk::DescriptorSet> descriptorSets;
	for (auto it = start; it != ordered.end(); it++)
		descriptorSets.push_back(it->second->GetDescriptorSet());

	if (!descriptorSets.empty())
		retVal.insert(std::make_pair(Enums::value(start->first), std::move(descriptorSets)));

	return retVal;
}

GraphicsPipelineCreateInfo::Specializations Material::SetupSpecializations() const
{
	static const std::map<vk::ImageType, int> s_TextureModeMap =
	{
		{ vk::ImageType::e1D, TEXTURE_MODE_1D },
		{ vk::ImageType::e2D, TEXTURE_MODE_2D },
		{ vk::ImageType::e3D, TEXTURE_MODE_3D },
	};

	GraphicsPipelineCreateInfo::Specializations retVal;

	const auto& shaderGroupData = m_Data->GetShaderGroup()->GetData();

	for (const auto& shaderParameter : shaderGroupData->GetParameters())
	{
		if (shaderParameter.second != ShaderParameterType::Texture)
			continue;

		for (const auto& dependentShader : shaderGroupData->FindByParameterDependency(shaderParameter.first))
		{
			assert(false);

			//const auto& found = dependentShader.

			//retVal[dependentShader.m_Definition->m_Type.value()][CID_TEXTURE_MODE_START + dependentShader.m_BindingIndex] =
			//	s_TextureModeMap.at(m_Textures.at(shaderParameter.first)->GetImageType());
		}
	}

	return retVal;
}
