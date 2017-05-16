#pragma once

struct DescriptorSetCreateInfo;
class GraphicsPipeline;
class LogicalDevice;

class DescriptorSet
{
public:
	DescriptorSet(LogicalDevice& device, const std::shared_ptr<const DescriptorSetCreateInfo>& createInfo);

	const DescriptorSetCreateInfo& GetCreateInfo() const { return *m_CreateInfo; }

	LogicalDevice& GetDevice() const { return m_Device; }

	vk::DescriptorSet GetDescriptorSet() const { return m_DescriptorSet.get(); }

	void Bind(uint32_t setID, const vk::CommandBuffer& cmdBuf, const GraphicsPipeline& pipeline) const;

private:
	void CreateDescriptorSet();

	std::shared_ptr<const DescriptorSetCreateInfo> m_CreateInfo;

	LogicalDevice& m_Device;
	vk::UniqueDescriptorSet m_DescriptorSet;
};