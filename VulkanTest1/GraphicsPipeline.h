#pragma once
#include "GraphicsPipelineCreateInfo.h"

#include <functional>

class GraphicsPipeline
{
public:
	static std::unique_ptr<GraphicsPipeline> Create(const std::shared_ptr<GraphicsPipelineCreateInfo>& createInfo);

	const GraphicsPipelineCreateInfo& GetCreateInfo() const { return *m_CreateInfo; }

	const vk::RenderPass GetRenderPass() const { return m_RenderPass.get(); }
	vk::RenderPass GetRenderPass() { return m_RenderPass.get(); }

	const vk::Pipeline GetPipeline() const { return m_Pipeline.get(); }
	vk::Pipeline GetPipeline() { return m_Pipeline.get(); }

private:
	GraphicsPipeline(const std::shared_ptr<const GraphicsPipelineCreateInfo>& createInfo);

	void CreateRenderPass();
	void CreatePipeline();
	static vk::ShaderStageFlagBits ConvertShaderType(ShaderType type);
	std::vector<vk::PipelineShaderStageCreateInfo> GenerateShaderStageCreateInfos() const;

	std::shared_ptr<const GraphicsPipelineCreateInfo> m_CreateInfo;

	vk::UniqueRenderPass m_RenderPass;
	vk::UniquePipelineLayout m_Layout;
	vk::UniquePipeline m_Pipeline;
};