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

		// See what textures this stage has
		for (const auto& texture : shaderModuleData->GetInputTextures())
		{
			// Check to make sure this texture is actually part of the SET_MATERIAL descriptor set.
			if (std::any_of(texture.second.m_Dimensions.begin(), texture.second.m_Dimensions.end(),
							[](const ShaderModuleData::InputVariable& a) { return a.m_SetID != SET_MATERIAL; }))
			{
				assert(std::none_of(texture.second.m_Dimensions.begin(), texture.second.m_Dimensions.end(),
									[](const ShaderModuleData::InputVariable& a) { return a.m_SetID == SET_MATERIAL; }));
				continue;
			}

			const auto found = GetData().GetInputs().find(texture.first);
			if (found == GetData().GetInputs().end())
			{
				Log::TagMsg(TAG, "Warning: Texture \"{0}\" in shader \"{1}\" of shader group \"{2}\" was not specified in material \"{3}\"",
						 texture.first, shaderModuleData->GetName(), GetData().GetShaderGroup().GetData().GetName(), GetData().GetName());
				continue;
			}

			const auto texPtr = TextureManager::Instance().Find(std::get<std::string>(found->second));
			assert(texPtr);
			if (!texPtr)
				continue;

			LayoutBinding newBinding;
			newBinding.m_Data = texPtr;
			newBinding.m_Binding.setDescriptorCount(1);
			newBinding.m_Binding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);

			for (const auto& dimension : texture.second.m_Dimensions)
			{
				if (texPtr->GetImageType() == vk::ImageType::e1D && dimension.m_Type.image.dim == spv::Dim::Dim1D ||
					texPtr->GetImageType() == vk::ImageType::e2D && dimension.m_Type.image.dim == spv::Dim::Dim2D ||
					texPtr->GetImageType() == vk::ImageType::e3D && dimension.m_Type.image.dim == spv::Dim::Dim3D)
				{
					newBinding.m_Binding.setBinding(dimension.m_BindingID);
					newBinding.m_FullName = dimension.m_FullName;
					newBinding.m_FriendlyName = dimension.m_FriendlyName;
					break;
				}
			}

			const auto& inserted = m_Bindings.insert(std::move(newBinding));
			const_cast<LayoutBinding&>(*inserted.first).m_Binding.stageFlags |= stageFlagBits;
		}
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
			outBinding.m_DebugName = inBinding.m_FullName;
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
	GraphicsPipelineCreateInfo::Specializations retVal;

	SetupTexModeSpecConstants(retVal);
	SetupParamSpecConstants(retVal);

	return retVal;
}

void Material::SetupTexModeSpecConstants(GraphicsPipelineCreateInfo::Specializations& retVal) const
{
	// Automatic _texMode spec constants
	for (const auto& texBinding : m_Bindings)
	{
		static const std::map<vk::ImageType, int> s_TextureModeMap =
		{
			{ vk::ImageType::e1D, TEXTURE_MODE_1D },
			{ vk::ImageType::e2D, TEXTURE_MODE_2D },
			{ vk::ImageType::e3D, TEXTURE_MODE_3D },
		};

		if (texBinding.m_Binding.descriptorType != vk::DescriptorType::eCombinedImageSampler)
			continue;	// Not a texture binding

		const auto wat = variant_type_index_v<std::shared_ptr<Texture>, decltype(texBinding.m_Data.value())>;
		assert(texBinding.m_Data.value().index() == wat);

		const std::string texModeConstantName = "_texMode_" + texBinding.m_FriendlyName;

		for (const auto& shaderModuleData : GetData().GetShaderGroup().GetData().GetShaderModulesData())
		{
			if (!shaderModuleData)
				continue;

			const auto found = shaderModuleData->GetInputSpecConstants().find(texModeConstantName);
			if (found != shaderModuleData->GetInputSpecConstants().end())
			{
				AssertAR(, retVal[shaderModuleData->GetType()].insert(std::make_pair(found->second.m_BindingID, s_TextureModeMap.at(std::get<std::shared_ptr<Texture>>(texBinding.m_Data.value())->GetImageType()))), .second);
			}
		}
	}
}

void Material::SetupParamSpecConstants(GraphicsPipelineCreateInfo::Specializations& retVal) const
{
	// User _param spec constants
	for (const auto& shaderModule : GetData().GetShaderGroup().GetData().GetShaderModulesData())
	{
		if (!shaderModule)
			continue;

		for (const auto& inputParam : GetData().GetInputs())
		{
			const auto found = shaderModule->GetInputSpecConstants().find(inputParam.first);
			if (found == shaderModule->GetInputSpecConstants().end())
				continue;

			using BaseType = spirv_cross::SPIRType::BaseType;
			using VariantType = std::decay_t<decltype(GetData().GetInputs())>::value_type::second_type;

			auto warningFn = [&](BaseType actualInputType)
			{
				Log::TagMsg(TAG, "Warning: Material \"{0}\" has input \"{1}\" of type {2}, but shader \"{3}\" expects that parameter to be of type {4}.", GetData().GetName(), found->first, actualInputType, shaderModule->GetName(), found->second.m_Type.basetype);
			};

			switch (inputParam.second.index())
			{
			case variant_type_index_v<bool, VariantType>:
			{
				if (found->second.m_Type.basetype == BaseType::Boolean)
					AssertAR(, retVal[shaderModule->GetType()].insert(std::make_pair(found->second.m_BindingID, std::get<bool>(inputParam.second))), .second);
				else
					warningFn(BaseType::Boolean);

				break;
			}
			case variant_type_index_v<double, VariantType>:
			{
				switch (found->second.m_Type.basetype)
				{
				case BaseType::Double:
				case BaseType::Float:
				case BaseType::Int:
				case BaseType::Int64:
				case BaseType::UInt:
				case BaseType::UInt64:
				{
					const auto& inserted = retVal[shaderModule->GetType()].insert(std::make_pair(found->second.m_BindingID, std::get<double>(inputParam.second)));
					assert(inserted.second);

					// Fucking broken assert macro
					constexpr auto index = variant_type_index_v<double, decltype(inserted.first->second)>;
					assert(inserted.first->second.index() == index);
					break;
				}

				default:
					warningFn(BaseType::Double);
				}
				break;
			}
			case variant_type_index_v<std::string, VariantType>:
				Log::TagMsg(TAG, "Warning: Material \"{0}\" has input \"{1}\" of type string, but shader \"{2}\" expects that parameter to be of type {3}.", GetData().GetName(), found->first, shaderModule->GetName(), found->second.m_Type.basetype);
				break;

			default:
				assert(false);
			}
		}
	}
}

size_t Material::LayoutBinding::hash::operator()(const LayoutBinding& x) const
{
	return std::hash<decltype(x.m_FullName)>{}(x.m_FullName);
}

bool Material::LayoutBinding::operator==(const LayoutBinding& rhs) const
{
	return (m_FullName == rhs.m_FullName &&
			m_Binding == rhs.m_Binding &&
			m_Data == rhs.m_Data);
}
