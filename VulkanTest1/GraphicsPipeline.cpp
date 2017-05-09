#include "stdafx.h"
#include "GraphicsPipeline.h"
#include "LogicalDevice.h"
#include "SimpleVertex.h"
#include "Swapchain.h"
#include "UniformBufferObject.h"
#include "Vulkan.h"

#include <glm/gtc/matrix_transform.hpp>

std::unique_ptr<GraphicsPipeline> GraphicsPipeline::Create(const std::shared_ptr<const GraphicsPipelineCreateInfo>& createInfo)
{
	return std::unique_ptr<GraphicsPipeline>(new GraphicsPipeline(createInfo));
}

std::vector<vk::DescriptorSet> GraphicsPipeline::GetDescriptorSets() const
{
	std::vector<vk::DescriptorSet> retVal;

	for (const auto& unique : m_DescriptorSets)
		retVal.push_back(unique.get());

	return retVal;
}

std::vector<vk::DescriptorSetLayout> GraphicsPipeline::GetDescriptorSetLayouts() const
{
	std::vector<vk::DescriptorSetLayout> retVal;

	for (const auto& unique : m_DescriptorSetLayouts)
		retVal.push_back(unique.get());

	return retVal;
}

void GraphicsPipeline::Update()
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float>(currentTime - startTime).count();

	m_UniformTimeBuffer->Write(&time, sizeof(time), 0);

	UniformBufferObject ubo;
	ubo.model = glm::rotate(glm::mat4(), time * glm::radians(90.0f), glm::vec3(0, 0, 1));
	ubo.view = glm::lookAt(glm::vec3(2, 2, 2), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));

	const auto& swapchainExtent = GetDevice().GetSwapchain().GetInitValues().m_Extent2D;
	ubo.proj = glm::perspective(glm::radians(45.0f), float(swapchainExtent.width) / swapchainExtent.height, 0.1f, 10.0f);

	m_UniformObjectBuffer->Write(&ubo, sizeof(ubo), 0);
}

GraphicsPipeline::GraphicsPipeline(const std::shared_ptr<const GraphicsPipelineCreateInfo>& createInfo)
{
	m_CreateInfo = createInfo;

	CreateDescriptorSetLayout();
	CreateUniformBuffer();
	CreateDescriptorPool();
	CreateDescriptorSet();
	CreateRenderPass();
	CreatePipeline();
}

void GraphicsPipeline::CreateDescriptorSetLayout()
{
	{
		vk::DescriptorSetLayoutBinding uboLayoutBinding;
		uboLayoutBinding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
		uboLayoutBinding.setDescriptorCount(1);
		uboLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eVertex);

		vk::DescriptorSetLayoutCreateInfo createInfo;
		createInfo.setBindingCount(1);
		createInfo.setPBindings(&uboLayoutBinding);

		m_DescriptorSetLayouts.push_back(GetDevice()->createDescriptorSetLayoutUnique(createInfo));
	}

	{
		vk::DescriptorSetLayoutBinding timeLayoutBinding;
		timeLayoutBinding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
		timeLayoutBinding.setDescriptorCount(1);
		timeLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

		vk::DescriptorSetLayoutCreateInfo createInfo;
		createInfo.setBindingCount(1);
		createInfo.setPBindings(&timeLayoutBinding);

		m_DescriptorSetLayouts.push_back(GetDevice()->createDescriptorSetLayoutUnique(createInfo));
	}
}

void GraphicsPipeline::CreateUniformBuffer()
{
	m_UniformObjectBuffer.emplace(GetDevice(), sizeof(UniformBufferObject), vk::BufferUsageFlagBits::eUniformBuffer,
								  vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	m_UniformTimeBuffer.emplace(GetDevice(), sizeof(float), vk::BufferUsageFlagBits::eUniformBuffer,
								vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
}

void GraphicsPipeline::CreateDescriptorPool()
{
	vk::DescriptorPoolSize poolSize;
	poolSize.setType(vk::DescriptorType::eUniformBuffer);
	poolSize.setDescriptorCount(10);

	vk::DescriptorPoolCreateInfo createInfo;
	createInfo.setPoolSizeCount(1);
	createInfo.setPPoolSizes(&poolSize);
	createInfo.setMaxSets(10);
	createInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

	m_DescriptorPool = GetDevice()->createDescriptorPoolUnique(createInfo);
}

void GraphicsPipeline::CreateDescriptorSet()
{
	const auto& descriptorSetLayouts = GetDescriptorSetLayouts();

	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.setDescriptorPool(m_DescriptorPool.get());
	allocInfo.setDescriptorSetCount(descriptorSetLayouts.size());
	allocInfo.setPSetLayouts(descriptorSetLayouts.data());

	m_DescriptorSets = GetDevice()->allocateDescriptorSetsUnique(allocInfo);

	std::vector<vk::DescriptorBufferInfo> bufferInfos;
	{
		{
			bufferInfos.emplace_back();
			vk::DescriptorBufferInfo& bufferInfo = bufferInfos.back();
			bufferInfo.setBuffer(m_UniformObjectBuffer->GetBuffer());
			bufferInfo.setRange(sizeof(UniformBufferObject));
		}
		{
			bufferInfos.emplace_back();
			vk::DescriptorBufferInfo& bufferInfo = bufferInfos.back();
			bufferInfo.setBuffer(m_UniformTimeBuffer->GetBuffer());
			bufferInfo.setRange(sizeof(float));
		}
	}

	std::vector<vk::WriteDescriptorSet> descriptorWrites;
	for (size_t i = 0; i < m_DescriptorSets.size(); i++)
	{
		descriptorWrites.emplace_back();
		vk::WriteDescriptorSet& descriptorWrite = descriptorWrites.back();

		descriptorWrite.setDstSet(m_DescriptorSets[i].get());
		descriptorWrite.setDescriptorType(vk::DescriptorType::eUniformBuffer);
		descriptorWrite.setDescriptorCount(1);
		descriptorWrite.setPBufferInfo(&bufferInfos[i]);
	}

	GetDevice()->updateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void GraphicsPipeline::CreateRenderPass()
{
	vk::AttachmentDescription colorAttachment;
	{
		colorAttachment.setFormat(m_CreateInfo->GetDevice().GetSwapchain().GetInitValues().m_SurfaceFormat.format);
		colorAttachment.setSamples(vk::SampleCountFlagBits::e1);

		colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
		colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);

		colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
		colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
		colorAttachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
	}

	vk::AttachmentReference colorAttachmentRef;
	{
		colorAttachmentRef.setAttachment(0);
		colorAttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	}

	vk::SubpassDescription subpass;
	{
		subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

		subpass.setColorAttachmentCount(1);
		subpass.setPColorAttachments(&colorAttachmentRef);
	}

	vk::SubpassDependency dependency;
	{
		dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
		dependency.setDstSubpass(0);

		dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		//dependency.setSrcAccessMask(0);

		dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);
	}

	vk::RenderPassCreateInfo rpCreateInfo;
	{
		rpCreateInfo.setAttachmentCount(1);
		rpCreateInfo.setPAttachments(&colorAttachment);
		rpCreateInfo.setSubpassCount(1);
		rpCreateInfo.setPSubpasses(&subpass);
		rpCreateInfo.setDependencyCount(1);
		rpCreateInfo.setPDependencies(&dependency);
	}

	const auto device = GetCreateInfo().GetDevice().Get();
	m_RenderPass = device.createRenderPassUnique(rpCreateInfo);
}

void GraphicsPipeline::CreatePipeline()
{
	vk::PipelineVertexInputStateCreateInfo vertexInputState;
	auto bindingDescription = SimpleVertex::GetBindingDescription();
	auto attributeDescriptions = SimpleVertex::GetAttributeDescriptions();
	vertexInputState.setVertexBindingDescriptionCount(1);
	vertexInputState.setPVertexBindingDescriptions(&bindingDescription);
	vertexInputState.setVertexAttributeDescriptionCount(attributeDescriptions.size());
	vertexInputState.setPVertexAttributeDescriptions(attributeDescriptions.data());

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
	inputAssemblyState.setTopology(vk::PrimitiveTopology::eTriangleList);

	const auto& swapchainExtent = m_CreateInfo->GetDevice().GetSwapchain().GetInitValues().m_Extent2D;

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

		rasterizationState.setCullMode(vk::CullModeFlagBits::eBack);
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

	const auto device = GetCreateInfo().GetDevice().Get();
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

		gpCreateInfo.setRenderPass(*m_RenderPass);
		gpCreateInfo.setSubpass(0);

		gpCreateInfo.setBasePipelineHandle(nullptr);
		gpCreateInfo.setBasePipelineIndex(-1);
	}

	m_Pipeline = device.createGraphicsPipelineUnique(nullptr, gpCreateInfo);
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
	for (std::underlying_type_t<ShaderType> i = 0; i < underlying_value(ShaderType::Count); i++)
	{
		const auto& current = m_CreateInfo->GetShader((ShaderType)i);
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
