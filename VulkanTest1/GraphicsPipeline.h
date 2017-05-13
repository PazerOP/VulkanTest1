#pragma once
#include "Buffer.h"
#include "GraphicsPipelineCreateInfo.h"
#include "ShaderType.h"

#include <functional>
#include <optional>

class GraphicsPipeline
{
public:
	GraphicsPipeline(LogicalDevice& device, const std::shared_ptr<const GraphicsPipelineCreateInfo>& createInfo);

	const GraphicsPipelineCreateInfo& GetCreateInfo() const { return *m_CreateInfo; }

	const LogicalDevice& GetDevice() const { return m_CreateInfo->GetDevice(); }
	LogicalDevice& GetDevice() { return const_cast<LogicalDevice&>(m_CreateInfo->GetDevice()); }	// dumb

	const vk::Pipeline GetPipeline() const { return m_Pipeline.get(); }
	vk::Pipeline GetPipeline() { return m_Pipeline.get(); }

	const vk::PipelineLayout GetPipelineLayout() const { return m_Layout.get(); }
	vk::PipelineLayout GetPipelineLayout() { return m_Layout.get(); }

	const std::vector<vk::DescriptorSet>& GetDescriptorSets() const { return m_DescriptorSets; }

private:
	void CreatePipeline();
	void InitDescriptorSets();
	static vk::ShaderStageFlagBits ConvertShaderType(ShaderType type);
	std::vector<vk::PipelineShaderStageCreateInfo> GenerateShaderStageCreateInfos() const;

	LogicalDevice& m_Device;

	std::shared_ptr<const GraphicsPipelineCreateInfo> m_CreateInfo;

	vk::UniquePipelineLayout m_Layout;
	vk::UniquePipeline m_Pipeline;

	std::vector<vk::UniqueDescriptorSet> m_UniqueDescriptorSetHandles;
	std::vector<vk::DescriptorSet> m_DescriptorSets;
	std::vector<Buffer> m_UniformBuffers;
};