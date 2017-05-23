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
	InitInputs();
	InitDescriptorSet();
	InitGraphicsPipeline();
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

void Material::InitInputs()
{
	// For each material input parameter
	for (const auto& inputParam : m_Data->GetInputs())
	{
		bool used = false;
		for (const auto& shaderModuleData : GetData().GetShaderGroup().GetData().GetShaderModulesData())
		{
			if (!shaderModuleData)
				continue;

			const auto& foundInputTextureData = shaderModuleData->GetInputTextures().find(inputParam.first);
			if (foundInputTextureData == shaderModuleData->GetInputTextures().end())
				continue;

			const auto& inputParamData = foundInputTextureData->second;

			const auto& texture = TextureManager::Instance().Find(inputParam.second);

			switch (inputParamData.m_Type.basetype)
			{
			case ShaderParameterType::SampledImage:
				m_Resources_[shaderModuleData->GetType()].m_Textures.insert(std::make_pair(
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
	m_Bindings.clear();

	for (const auto& shaderModuleData : GetData().GetShaderGroup().GetData().GetShaderModulesData())
	{
		if (!shaderModuleData)
			continue;

		const auto stageFlagBits = Enums::convert<vk::ShaderStageFlagBits>(shaderModuleData->GetType());

		// See what parameters this stage has
		for (const auto& texture : shaderModuleData->GetInputTextures())
		{
			if (texture.second.m_SetID != SET_MATERIAL)
				continue;

			const auto texPtr = TextureManager::Instance().Find(GetData().GetInputs().at(texture.second.m_FriendlyName));
			assert(texPtr);
			if (!texPtr)
				continue;

			LayoutBinding newBinding;

			if (texPtr->GetImageType() == vk::ImageType::e1D)
			{
				texture.second.m_Type.image.dim == spv::Dim::Dim1D
			}

			newBinding.m_Name =
		}
#if 0
		for (const auto& param : shaderModuleData->GetInputVariables())
		{
			if (param.second.m_SetID != SET_MATERIAL)
				continue;

			LayoutBinding newBinding;
			newBinding.m_Name = param.first;
			newBinding.m_Binding.setDescriptorCount(1);
			newBinding.m_Binding.setBinding(param.second.m_BindingID);
			newBinding.m_Binding.setDescriptorType(param.second.m_DescriptorType);

			switch (newBinding.m_Binding.descriptorType)
			{
			case vk::DescriptorType::eCombinedImageSampler:
			{
				static const std::string s_TexPrefixes[] =
				{
					"_tex1D_"s,
					"_tex2D_"s,
					"_tex3D_"s
				};

				std::string friendlyName;
				for (const auto& prefix : s_TexPrefixes)
				{
					if (StringTools::BeginsWith(param.first, prefix))
						friendlyName = param.first.substr(prefix.size());
				}

				const auto& value = GetData().GetInputs().at(friendlyName);

				newBinding.m_Data = TextureManager::Instance().Find(value);
				break;
			}

			case vk::DescriptorType::eUniformBuffer:

			default:
				assert(!"Fix this code");
			}

			// We are OK doing this const_cast because we're only touching the stageFlags, which
			// are not part of the set's key.

			const auto& inserted = m_Bindings.insert(std::move(newBinding));
			const_cast<LayoutBinding&>(*inserted.first).m_Binding.stageFlags |= stageFlagBits;
		}
#endif
	}

	// Descriptor set layout
	{
		auto layoutCreateInfo = std::make_shared<DescriptorSetLayoutCreateInfo>();
		layoutCreateInfo->m_DebugName = __FUNCSIG__;

		for (const auto& binding : m_Bindings)
			layoutCreateInfo->m_Bindings.push_back(binding.m_Binding);

		m_DescriptorSetLayout = std::make_shared<DescriptorSetLayout>(m_Device, layoutCreateInfo);
	}

	// Descriptor set
	{
		auto setCreateInfo = std::make_shared<DescriptorSetCreateInfo>();
		setCreateInfo->m_Layout = m_DescriptorSetLayout;

		for (const auto& inBinding : m_Bindings)
		{
			setCreateInfo->m_Data.emplace_back();
			DescriptorSetCreateInfo::Binding& outBinding = setCreateInfo->m_Data.back();

			outBinding.m_BindingIndex = inBinding.m_Binding.binding;
			outBinding.m_DebugName = inBinding.m_Name;
			outBinding.m_Stages = inBinding.m_Binding.stageFlags;
			outBinding.m_Data = inBinding.m_Data.value();
		}

		m_DescriptorSet = std::make_shared<DescriptorSet>(m_Device, setCreateInfo);
	}
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

	throw not_implemented_error();

#if 0
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
#endif
}

size_t Material::LayoutBinding::hash::operator()(const LayoutBinding& x) const
{
	return std::hash<decltype(x.m_Name)>{}(x.m_Name);
}

bool Material::LayoutBinding::operator==(const LayoutBinding& rhs) const
{
	return (m_Name == rhs.m_Name &&
			m_Binding == rhs.m_Binding &&
			m_Data == rhs.m_Data);
}
