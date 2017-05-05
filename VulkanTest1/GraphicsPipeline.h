#pragma once
#include "GraphicsPipelineCreateInfo.h"

class GraphicsPipeline
{
public:
	GraphicsPipeline(const std::shared_ptr<const GraphicsPipelineCreateInfo>& createInfo);

	const std::shared_ptr<const GraphicsPipelineCreateInfo>& GetCreateInfo() const { return m_CreateInfo; }

	const std::shared_ptr<const vk::RenderPass> GetRenderPass() const { return m_RenderPass; }
	const std::shared_ptr<vk::RenderPass>& GetRenderPass() { return m_RenderPass; }

private:
	void CreateRenderPass();
	void CreatePipeline();
	static vk::ShaderStageFlagBits ConvertShaderType(ShaderType type);
	std::vector<vk::PipelineShaderStageCreateInfo> GenerateShaderStageCreateInfos() const;

	std::shared_ptr<const GraphicsPipelineCreateInfo> m_CreateInfo;

	std::shared_ptr<vk::RenderPass> m_RenderPass;
	std::shared_ptr<vk::PipelineLayout> m_Layout;
	std::shared_ptr<vk::Pipeline> m_Pipeline;
};