#pragma once
#include "Buffer.h"
#include "GraphicsPipelineCreateInfo.h"

#include <functional>
#include <optional>

class GraphicsPipeline
{
public:
	static std::unique_ptr<GraphicsPipeline> Create(const std::shared_ptr<const GraphicsPipelineCreateInfo>& createInfo);

	const GraphicsPipelineCreateInfo& GetCreateInfo() const { return *m_CreateInfo; }

	const LogicalDevice& GetDevice() const { return m_CreateInfo->GetDevice(); }
	LogicalDevice& GetDevice() { return const_cast<LogicalDevice&>(m_CreateInfo->GetDevice()); }	// dumb

	const vk::RenderPass GetRenderPass() const { return m_RenderPass.get(); }
	vk::RenderPass GetRenderPass() { return m_RenderPass.get(); }

	const vk::Pipeline GetPipeline() const { return m_Pipeline.get(); }
	vk::Pipeline GetPipeline() { return m_Pipeline.get(); }

	const vk::PipelineLayout GetPipelineLayout() const { return m_Layout.get(); }
	vk::PipelineLayout GetPipelineLayout() { return m_Layout.get(); }

	std::vector<vk::DescriptorSet> GetDescriptorSets() const;
	std::vector<vk::DescriptorSetLayout> GetDescriptorSetLayouts() const;

	void Update();

private:
	GraphicsPipeline(const std::shared_ptr<const GraphicsPipelineCreateInfo>& createInfo);

	void CreateDescriptorSetLayout();
	void CreateUniformBuffer();
	void CreateDescriptorPool();
	void CreateDescriptorSet();
	void CreateRenderPass();
	void CreatePipeline();
	static vk::ShaderStageFlagBits ConvertShaderType(ShaderType type);
	std::vector<vk::PipelineShaderStageCreateInfo> GenerateShaderStageCreateInfos() const;

	std::shared_ptr<const GraphicsPipelineCreateInfo> m_CreateInfo;

	vk::UniqueRenderPass m_RenderPass;
	vk::UniquePipelineLayout m_Layout;
	vk::UniquePipeline m_Pipeline;

	std::vector<vk::UniqueDescriptorSetLayout> m_DescriptorSetLayouts;
	vk::UniqueDescriptorPool m_DescriptorPool;
	std::vector<vk::UniqueDescriptorSet> m_DescriptorSets;
	std::optional<Buffer> m_UniformObjectBuffer;
	std::optional<Buffer> m_UniformTimeBuffer;
};