#pragma once
#include "GraphicsPipeline.h"
#include "IMaterial.h"

#include <filesystem>

class LogicalDevice;
class MaterialData;

class Material : public IMaterial
{
public:
	Material(const std::shared_ptr<const MaterialData>& data, LogicalDevice& device);

	virtual void Bind(const vk::CommandBuffer& cmdBuf) const override;

	const GraphicsPipeline& GetPipeline() const { return m_GraphicsPipeline.value(); }
	GraphicsPipeline& GetPipeline() { return m_GraphicsPipeline.value(); }

private:
	LogicalDevice& m_Device;
	std::shared_ptr<const MaterialData> m_Data;

	std::vector<vk::DescriptorSet> GetDescriptorSets() const;

	std::optional<GraphicsPipeline> m_GraphicsPipeline;
};