#pragma once
#include <optional>
#include <vulkan/vulkan.hpp>

class DescriptorSetLayout;
class ShaderGroup;

struct GraphicsPipelineCreateInfo
{
	std::shared_ptr<const ShaderGroup> m_ShaderGroup;
	std::vector<std::shared_ptr<DescriptorSetLayout>> m_DescriptorSetLayouts;

	std::optional<vk::VertexInputBindingDescription> m_VertexInputBindingDescription;
	std::vector<vk::VertexInputAttributeDescription> m_VertexInputAttributeDescriptions;
};