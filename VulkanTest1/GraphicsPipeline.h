#pragma once
#include "Buffer.h"
#include "GraphicsPipelineCreateInfo.h"
#include "ShaderType.h"

#include <forward_list>
#include <functional>
#include <optional>
#include <variant>

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

	void RecreatePipeline();

private:
	struct ShaderStageData
	{
		ShaderStageData() = default;
		ShaderStageData(const ShaderStageData& other) = delete;
		ShaderStageData(ShaderStageData&& other) = delete;
		std::vector<vk::PipelineShaderStageCreateInfo> m_StageCreateInfos;
		std::forward_list<std::pair<vk::SpecializationInfo, std::vector<vk::SpecializationMapEntry>>> m_SpecializationInfoStorage;
		std::vector<std::variant<vk::Bool32, int, float>> m_Storage;
	};

	void CreatePipeline();
	void GenerateShaderStageCreateInfos(ShaderStageData& data) const;

	LogicalDevice& m_Device;

	std::vector<vk::DescriptorSetLayout> GetDescriptorSetLayouts() const;

	std::shared_ptr<const GraphicsPipelineCreateInfo> m_CreateInfo;

	vk::UniquePipelineLayout m_Layout;
	vk::UniquePipeline m_Pipeline;
};