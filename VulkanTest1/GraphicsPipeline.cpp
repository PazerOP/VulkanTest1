#include "stdafx.h"
#include "GraphicsPipeline.h"

#include "DescriptorSet.h"
#include "DescriptorSetCreateInfo.h"
#include "DescriptorSetLayout.h"
#include "LogicalDevice.h"
#include "SimpleVertex.h"
#include "ShaderGroup.h"
#include "ShaderModule.h"
#include "Swapchain.h"
#include "Vulkan.h"

GraphicsPipeline::GraphicsPipeline(LogicalDevice& device, const std::shared_ptr<const GraphicsPipelineCreateInfo>& createInfo) :
	m_Device(device), m_CreateInfo(createInfo)
{
	Log::Msg<LogType::ObjectLifetime>(__FUNCSIG__);

	if (!m_CreateInfo->m_ShaderGroup)
		throw std::invalid_argument("Attempted to create a GraphicsPipeline object with a GraphicsPipelineCreateInfo that did not have a ShaderGroup set.");
	if (!m_CreateInfo->m_VertexInputBindingDescription.has_value())
		throw std::invalid_argument("Attempted to create a GraphicsPipeline object with a GraphicsPipelineCreateInfo that did not have a VertexInputBindingDescription specified.");
	if (m_CreateInfo->m_VertexInputAttributeDescriptions.empty())
		throw std::invalid_argument("Attempted to create a GraphicsPipeline object with a GraphicsPipelineCreateInfo that did not have any VertexInputAttributeDescriptions specified.");

	CreatePipeline();
}

void GraphicsPipeline::RecreatePipeline()
{
	CreatePipeline();
}

void GraphicsPipeline::CreatePipeline()
{
	vk::PipelineVertexInputStateCreateInfo vertexInputState;
	vertexInputState.setVertexBindingDescriptionCount(1);
	vertexInputState.setPVertexBindingDescriptions(&m_CreateInfo->m_VertexInputBindingDescription.value());
	vertexInputState.setVertexAttributeDescriptionCount(m_CreateInfo->m_VertexInputAttributeDescriptions.size());
	vertexInputState.setPVertexAttributeDescriptions(m_CreateInfo->m_VertexInputAttributeDescriptions.data());

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
	inputAssemblyState.setTopology(vk::PrimitiveTopology::eTriangleList);

	const auto& swapchainExtent = GetDevice().GetSwapchain().GetInitValues().m_Extent2D;

	vk::Viewport viewport;
	viewport.setWidth((float)swapchainExtent.width);
	viewport.setHeight((float)swapchainExtent.height);
	viewport.setMaxDepth(1);

	vk::Rect2D scissor(vk::Offset2D(), swapchainExtent);

	vk::PipelineViewportStateCreateInfo viewportState;
	viewportState.setViewportCount(1);
	viewportState.setPViewports(&viewport);
	viewportState.setScissorCount(1);
	viewportState.setPScissors(&scissor);

	vk::PipelineRasterizationStateCreateInfo rasterizationState;
	{
		rasterizationState.setDepthClampEnable(false);

		rasterizationState.setRasterizerDiscardEnable(false);

		rasterizationState.setPolygonMode(vk::PolygonMode::eFill);

		rasterizationState.setLineWidth(1);

		rasterizationState.setCullMode(vk::CullModeFlagBits::eNone);
		rasterizationState.setFrontFace(vk::FrontFace::eClockwise);

		rasterizationState.setDepthBiasEnable(false);
	}

	vk::PipelineMultisampleStateCreateInfo multisampleState;
	{
		multisampleState.setSampleShadingEnable(false);
		multisampleState.setRasterizationSamples(vk::SampleCountFlagBits::e1);
		multisampleState.setMinSampleShading(1);
		multisampleState.setPSampleMask(nullptr);
		multisampleState.setAlphaToCoverageEnable(false);
		multisampleState.setAlphaToOneEnable(false);
	}

	vk::PipelineColorBlendAttachmentState pcbaState;
	{
		pcbaState.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
		pcbaState.setBlendEnable(false);
		pcbaState.setSrcColorBlendFactor(vk::BlendFactor::eOne);
		pcbaState.setDstColorBlendFactor(vk::BlendFactor::eZero);
		pcbaState.setColorBlendOp(vk::BlendOp::eAdd);
		pcbaState.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
		pcbaState.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
	}

	vk::PipelineColorBlendStateCreateInfo colorBlendState;
	{
		colorBlendState.setLogicOpEnable(false);
		colorBlendState.setLogicOp(vk::LogicOp::eCopy);
		colorBlendState.setAttachmentCount(1);
		colorBlendState.setPAttachments(&pcbaState);
	}

	const vk::DynamicState dynamicStates[] =
	{
		vk::DynamicState::eViewport,
		vk::DynamicState::eLineWidth,
	};

	vk::PipelineDynamicStateCreateInfo dynamicState;
	{
		dynamicState.setDynamicStateCount(std::size(dynamicStates));
		dynamicState.setPDynamicStates(dynamicStates);
	}

	{
		vk::PipelineLayoutCreateInfo plCreateInfo;
		const auto& descriptorSetLayouts = GetDescriptorSetLayouts();
		plCreateInfo.setSetLayoutCount(descriptorSetLayouts.size());
		plCreateInfo.setPSetLayouts(descriptorSetLayouts.data());

		m_Layout = GetDevice()->createPipelineLayoutUnique(plCreateInfo);
	}

	ShaderStageData shaderStages;
	GenerateShaderStageCreateInfos(shaderStages);
	vk::GraphicsPipelineCreateInfo gpCreateInfo;
	{
		gpCreateInfo.setStageCount(shaderStages.m_StageCreateInfos.size());
		gpCreateInfo.setPStages(shaderStages.m_StageCreateInfos.data());

		gpCreateInfo.setPVertexInputState(&vertexInputState);
		gpCreateInfo.setPInputAssemblyState(&inputAssemblyState);
		gpCreateInfo.setPViewportState(&viewportState);
		gpCreateInfo.setPRasterizationState(&rasterizationState);
		gpCreateInfo.setPMultisampleState(&multisampleState);
		gpCreateInfo.setPDepthStencilState(nullptr);
		gpCreateInfo.setPColorBlendState(&colorBlendState);
		//gpCreateInfo.setPDynamicState(&dynamicState);

		gpCreateInfo.setLayout(*m_Layout);

		gpCreateInfo.setRenderPass(m_Device.GetRenderPass());
		gpCreateInfo.setSubpass(0);

		gpCreateInfo.setBasePipelineHandle(nullptr);
		gpCreateInfo.setBasePipelineIndex(-1);
	}

	m_Pipeline = GetDevice()->createGraphicsPipelineUnique(nullptr, gpCreateInfo);
}

void GraphicsPipeline::GenerateShaderStageCreateInfos(ShaderStageData& data) const
{
	const auto& inputSpecializations = m_CreateInfo->m_Specializations;

	for (std::underlying_type_t<ShaderType> i = 0; i < Enums::count<ShaderType>(); i++)
	{
		const auto& current = m_CreateInfo->m_ShaderGroup->GetModule(Enums::index_to_value<ShaderType>(i));
		if (!current)
			continue;

		data.m_StageCreateInfos.emplace_back();
		auto& currentInfo = data.m_StageCreateInfos.back();

		currentInfo.setStage(Enums::convert<vk::ShaderStageFlagBits>((ShaderType)i));
		currentInfo.setModule(current->Get());
		currentInfo.setPName("main");

		const auto& foundInputSpecializations = inputSpecializations.find(ShaderType(i));
		if (foundInputSpecializations != inputSpecializations.end())
		{
			data.m_SpecializationInfoStorage.emplace_front();
			auto& outputSpecializations = data.m_SpecializationInfoStorage.front();

			for (const auto& inputSpecialization : foundInputSpecializations->second)
			{
				outputSpecializations.second.emplace_back();
				vk::SpecializationMapEntry& outputEntry = outputSpecializations.second.back();
				outputEntry.setConstantID(inputSpecialization.first);

				switch (inputSpecialization.second.index())
				{
				case 0:
				{
					data.m_Storage.push_back((vk::Bool32)std::get<bool>(inputSpecialization.second));
					outputEntry.setSize(sizeof(vk::Bool32));
					outputEntry.setOffset((char*)&std::get<vk::Bool32>(data.m_Storage.back()) - (char*)&data.m_Storage.front());
					break;
				}
				case 1:
				{
					data.m_Storage.push_back(std::get<int>(inputSpecialization.second));
					outputEntry.setSize(sizeof(int));
					outputEntry.setOffset((char*)&std::get<int>(data.m_Storage.back()) - (char*)&data.m_Storage.front());
					break;
				}
				case 2:
				{
					data.m_Storage.push_back(std::get<float>(inputSpecialization.second));
					outputEntry.setSize(sizeof(float));
					outputEntry.setOffset((char*)&std::get<float>(data.m_Storage.back()) - (char*)&data.m_Storage.front());
					break;
				}
				}
			}
			outputSpecializations.first.setDataSize(data.m_Storage.size() * sizeof(decltype(data.m_Storage)::value_type));

#pragma message("warning: Evil, evil, EVIL hack brought about by sleep deprivation: this pointer will become invalidated when the vector reallocates! need to have our own vector per specializationInfo!")
			outputSpecializations.first.setPData(data.m_Storage.data());

			outputSpecializations.first.setMapEntryCount(outputSpecializations.second.size());
			outputSpecializations.first.setPMapEntries(outputSpecializations.second.data());

			currentInfo.setPSpecializationInfo(&outputSpecializations.first);
		}
	}
}

std::vector<vk::DescriptorSetLayout> GraphicsPipeline::GetDescriptorSetLayouts() const
{
	// Potentially non-contiguous
	uint32_t highest = std::prev(m_CreateInfo->m_DescriptorSetLayouts.end())->first;

	std::vector<vk::DescriptorSetLayout> retVal(highest + 1);
	for (const auto& descSet : m_CreateInfo->m_DescriptorSetLayouts)
		retVal[descSet.first] = descSet.second->Get();

	return retVal;
}
