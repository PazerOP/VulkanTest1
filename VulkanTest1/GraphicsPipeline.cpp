#include "stdafx.h"
#include "GraphicsPipeline.h"
#include "LogicalDevice.h"
#include "Swapchain.h"

#include "Vulkan.h"

GraphicsPipeline::GraphicsPipeline(const std::shared_ptr<const GraphicsPipelineCreateInfo>& createInfo)
{
	m_CreateInfo = createInfo;

	CreateRenderPass();
	CreatePipeline();
}

void GraphicsPipeline::CreateRenderPass()
{
	vk::AttachmentDescription colorAttachment;
	{
		colorAttachment.setFormat(m_CreateInfo->GetDevice()->GetSwapchain()->GetInitValues()->m_SurfaceFormat.format);
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

	const auto device = m_CreateInfo->GetDevice();
	m_RenderPass = std::shared_ptr<vk::RenderPass>(
		new vk::RenderPass(device->Get().createRenderPass(rpCreateInfo)),
		[device](vk::RenderPass* rp) { device->Get().destroyRenderPass(*rp); delete rp; }
	);
}

void GraphicsPipeline::CreatePipeline()
{
	vk::PipelineVertexInputStateCreateInfo vertexInputState;

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
	inputAssemblyState.setTopology(vk::PrimitiveTopology::eTriangleList);

	const auto& swapchainExtent = m_CreateInfo->GetDevice()->GetSwapchain()->GetInitValues()->m_Extent2D;

	vk::Viewport viewport;
	viewport.setWidth(swapchainExtent.width);
	viewport.setHeight(swapchainExtent.height);
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

	const auto device = m_CreateInfo->GetDevice();
	m_Layout = std::shared_ptr<vk::PipelineLayout>(
		new vk::PipelineLayout(device->Get().createPipelineLayout(plCreateInfo)),
		[device](vk::PipelineLayout* pl) { device->Get().destroyPipelineLayout(*pl); delete pl; }
	);

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

	m_Pipeline = std::shared_ptr<vk::Pipeline>(
		new vk::Pipeline(device->Get().createGraphicsPipeline(nullptr, gpCreateInfo)),
		[device](vk::Pipeline* rp) { device->Get().destroyPipeline(*rp); delete rp; }
	);
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

		currentInfo.setModule(*current->GetShader());
		currentInfo.setPName("main");
	}
	return createInfos;
}
