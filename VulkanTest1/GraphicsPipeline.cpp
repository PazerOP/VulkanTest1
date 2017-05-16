#include "stdafx.h"
#include "GraphicsPipeline.h"

#include "DescriptorSet.h"
#include "DescriptorSetCreateInfo.h"
#include "DescriptorSetLayout.h"
#include "LogicalDevice.h"
#include "SimpleVertex.h"
#include "ShaderGroup.h"
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

	InitDescriptorSets();
	CreatePipeline();
}

const std::vector<std::shared_ptr<const DescriptorSet>>& GraphicsPipeline::GetDescriptorSets() const
{
	return reinterpret_cast<const std::vector<std::shared_ptr<const DescriptorSet>>&>(m_DescriptorSets);
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

	vk::PipelineLayoutCreateInfo plCreateInfo;
	const auto& descriptorSetLayouts = GetDescriptorSetLayouts();
	plCreateInfo.setSetLayoutCount(descriptorSetLayouts.size());
	plCreateInfo.setPSetLayouts(descriptorSetLayouts.data());

	const auto device = GetDevice().Get();
	m_Layout = device.createPipelineLayoutUnique(plCreateInfo);

	const auto shaderStages = GenerateShaderStageCreateInfos();
	vk::GraphicsPipelineCreateInfo gpCreateInfo;
	{
		gpCreateInfo.setStageCount(shaderStages.size());
		gpCreateInfo.setPStages(shaderStages.data());

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

	m_Pipeline = device.createGraphicsPipelineUnique(nullptr, gpCreateInfo);
}

void GraphicsPipeline::InitDescriptorSets()
{
	m_DescriptorSets = GetDevice().GetBuiltinUniformBuffers().GetDescriptorSets();
}

vk::ShaderStageFlagBits GraphicsPipeline::ConvertShaderType(ShaderType type)
{
	switch (type)
	{
	case ShaderType::Vertex:					return vk::ShaderStageFlagBits::eVertex;

	case ShaderType::TessellationControl:		return vk::ShaderStageFlagBits::eTessellationControl;
	case ShaderType::TessellationEvaluation:	return vk::ShaderStageFlagBits::eTessellationEvaluation;

	case ShaderType::Gemoetry:					return vk::ShaderStageFlagBits::eGeometry;

	case ShaderType::Compute:					return vk::ShaderStageFlagBits::eCompute;

	case ShaderType::Fragment:					return vk::ShaderStageFlagBits::eFragment;

	default:
		assert(!"Invalid ShaderType!");
		return (vk::ShaderStageFlagBits)0;
	}
}

std::vector<vk::PipelineShaderStageCreateInfo> GraphicsPipeline::GenerateShaderStageCreateInfos() const
{
	std::vector<vk::PipelineShaderStageCreateInfo> createInfos;
	for (std::underlying_type_t<ShaderType> i = 0; i < Enums::count<ShaderType>(); i++)
	{
		const auto& current = m_CreateInfo->m_ShaderGroup->GetModule(Enums::index_to_value<ShaderType>(i));
		if (!current)
			continue;

		createInfos.emplace_back();
		auto& currentInfo = createInfos.back();

		currentInfo.setStage(ConvertShaderType((ShaderType)i));

		currentInfo.setModule(current->Get());
		currentInfo.setPName("main");
	}
	return createInfos;
}

std::vector<vk::DescriptorSetLayout> GraphicsPipeline::GetDescriptorSetLayouts() const
{
	std::vector<vk::DescriptorSetLayout> retVal;
	for (const auto& descSet : m_CreateInfo->m_DescriptorSetLayouts)
		retVal.push_back(descSet->Get());

	return retVal;
}
