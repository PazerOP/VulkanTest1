#pragma once
#include <vulkan/vulkan.hpp>

struct DescriptorSetLayoutCreateInfo;
class LogicalDevice;

// A wrapper around a descriptor set layout, an object which defines
// *how* data is bound to a shader (but doesn't contain the data itself).
class DescriptorSetLayout
{
public:
	DescriptorSetLayout(LogicalDevice& device, const std::shared_ptr<const DescriptorSetLayoutCreateInfo>& createInfo);

	const std::shared_ptr<const DescriptorSetLayoutCreateInfo>& GetCreateInfo() const { return m_CreateInfo; }

	operator vk::DescriptorSetLayout() const { return m_Layout.get(); }
	vk::DescriptorSetLayout Get() const { return m_Layout.get(); }

	LogicalDevice& GetDevice() const { return m_Device; }

private:
	void CreateDescriptorSetLayout();

	LogicalDevice& m_Device;

	std::shared_ptr<const DescriptorSetLayoutCreateInfo> m_CreateInfo;
	vk::UniqueDescriptorSetLayout m_Layout;
};