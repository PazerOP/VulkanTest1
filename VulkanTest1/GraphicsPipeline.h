#pragma once
#include "Buffer.h"
#include "GraphicsPipelineCreateInfo.h"
#include "ShaderType.h"

#include <functional>
#include <optional>

class DescriptorSet;

class GraphicsPipeline
{
public:
	GraphicsPipeline(LogicalDevice& device, const std::shared_ptr<const GraphicsPipelineCreateInfo>& createInfo);

	const GraphicsPipelineCreateInfo& GetCreateInfo() const { return *m_CreateInfo; }

	LogicalDevice& GetDevice() const { return m_Device; }

	const vk::Pipeline GetPipeline() const { return m_Pipeline.get(); }
	vk::Pipeline GetPipeline() { return m_Pipeline.get(); }

	const vk::PipelineLayout GetPipelineLayout() const { return m_Layout.get(); }
	vk::PipelineLayout GetPipelineLayout() { return m_Layout.get(); }

	const std::vector<std::shared_ptr<const DescriptorSet>>& GetDescriptorSets() const;
	const std::vector<std::shared_ptr<DescriptorSet>>& GetDescriptorSets() { return m_DescriptorSets; }

	void RecreatePipeline();

private:
	void CreatePipeline();
	void InitDescriptorSets();
	static vk::ShaderStageFlagBits ConvertShaderType(ShaderType type);
	std::vector<vk::PipelineShaderStageCreateInfo> GenerateShaderStageCreateInfos() const;

	LogicalDevice& m_Device;

	std::vector<vk::DescriptorSetLayout> GetDescriptorSetLayouts() const;

	std::shared_ptr<const GraphicsPipelineCreateInfo> m_CreateInfo;

	vk::UniquePipelineLayout m_Layout;
	vk::UniquePipeline m_Pipeline;

	std::vector<vk::UniqueDescriptorSet> m_UniqueDescriptorSetHandles;
	std::vector<std::shared_ptr<DescriptorSet>> m_DescriptorSets;
};