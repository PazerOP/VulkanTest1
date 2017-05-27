#include "stdafx.h"
#include "GraphicsPipeline.h"

#include "DescriptorSet.h"
#include "DescriptorSetCreateInfo.h"
#include "DescriptorSetLayout.h"
#include "LogicalDevice.h"
#include "SimpleVertex.h"
#include "ShaderGroup.h"
#include "ShaderGroupData.h"
#include "ShaderModule.h"
#include "ShaderModuleData.h"
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
#if true
		pcbaState.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
		pcbaState.setBlendEnable(true);
		pcbaState.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
		pcbaState.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
		pcbaState.setColorBlendOp(vk::BlendOp::eAdd);
		pcbaState.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
		pcbaState.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
		pcbaState.setAlphaBlendOp(vk::BlendOp::eAdd);
#else
		pcbaState.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
		pcbaState.setBlendEnable(false);
		pcbaState.setSrcColorBlendFactor(vk::BlendFactor::eOne);
		pcbaState.setDstColorBlendFactor(vk::BlendFactor::eZero);
		pcbaState.setColorBlendOp(vk::BlendOp::eAdd);
		pcbaState.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
		pcbaState.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
		pcbaState.setAlphaBlendOp(vk::BlendOp::eAdd);
#endif
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

template<class T>
inline void GraphicsPipeline::ShaderStageData::SpecializationInfo::InsertData(const T& input, vk::SpecializationMapEntry& entry)
{
	m_Storage.push_back(input);
	entry.setSize(sizeof(T));

	const auto offset = (char*)&std::get<T>(m_Storage.back()) - (char*)&m_Storage.front();
	assert(offset < std::numeric_limits<uint32_t>::max());
	entry.setOffset(uint32_t(offset));
}

void GraphicsPipeline::GenerateShaderStageCreateInfos(ShaderStageData& data) const
{
	const auto& inputSpecializations = m_CreateInfo->m_Specializations;

	for (std::underlying_type_t<ShaderType> i = 0; i < Enums::count<ShaderType>(); i++)
	{
		const auto& current = m_CreateInfo->m_ShaderGroup->GetModulePtr(Enums::index_to_value<ShaderType>(i));
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
			const auto& allSpecConstantsData = GetCreateInfo().m_ShaderGroup->GetData().GetShaderModulesData().at(i)->GetInputSpecConstants();

			data.m_SpecializationInfos.emplace_front();
			auto& stageSpecInfo = data.m_SpecializationInfos.front();

			for (const auto& inputSpecialization : foundInputSpecializations->second)
			{
				stageSpecInfo.m_MapEntries.emplace_back();
				vk::SpecializationMapEntry& outputEntry = stageSpecInfo.m_MapEntries.back();
				outputEntry.setConstantID(inputSpecialization.first);

				const auto& thisSpecConstantData = std::find_if(allSpecConstantsData.begin(), allSpecConstantsData.end(),
					[&](const auto& pair) { return pair.second.m_BindingID == inputSpecialization.first; })->second;

				using BaseType = spirv_cross::SPIRType::BaseType;

				// Deal with implicit casts here
				switch (inputSpecialization.second.index())
				{
				case variant_type_index_v<bool, decltype(inputSpecialization.second)>:
				{
					if (thisSpecConstantData.m_Type.basetype != BaseType::Boolean)
						throw ConflictingSpecConstTypeException(typeid(bool), thisSpecConstantData.m_Type.basetype, inputSpecialization.first, ShaderType(i));

					stageSpecInfo.InsertData((vk::Bool32)std::get<bool>(inputSpecialization.second), outputEntry);
					break;
				}
				case variant_type_index_v<int32_t, decltype(inputSpecialization.second)>:
				{
					stageSpecInfo.InsertData(std::get<int32_t>(inputSpecialization.second), outputEntry);
					break;
				}
				case variant_type_index_v<float, decltype(inputSpecialization.second)>:
				{

					stageSpecInfo.InsertData(std::get<float>(inputSpecialization.second), outputEntry);
					break;
				}

				default:
					assert(false);
				}
			}
			stageSpecInfo.m_Info.setDataSize(stageSpecInfo.m_Storage.size() * sizeof(decltype(stageSpecInfo.m_Storage)::value_type));

			stageSpecInfo.m_Info.setPData(stageSpecInfo.m_Storage.data());

			stageSpecInfo.m_Info.setMapEntryCount(stageSpecInfo.m_MapEntries.size());
			stageSpecInfo.m_Info.setPMapEntries(stageSpecInfo.m_MapEntries.data());

			currentInfo.setPSpecializationInfo(&stageSpecInfo.m_Info);
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

GraphicsPipeline::ConflictingSpecConstTypeException::ConflictingSpecConstTypeException(const std::type_info& inputType, BaseType outputType, uint32_t index, ShaderType type) :
	Exception("ConflictingSpecializationTypeException", GenerateWhat(inputType, outputType, index, type))
{
}

std::string GraphicsPipeline::ConflictingSpecConstTypeException::GenerateWhat(const std::type_info& inputType, BaseType outputType, uint32_t index, ShaderType type)
{
	return StringTools::CSFormat("Unable to implicitly convert between input data of type {0} and output type {1} for {2} shader specialization constant {3}.",
		inputType.name(), outputType, index, type);
}
