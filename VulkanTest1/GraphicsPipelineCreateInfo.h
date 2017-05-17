#pragma once
#include <map>
#include <optional>
#include <vulkan/vulkan.hpp>

class DescriptorSetLayout;
class ShaderGroup;

struct GraphicsPipelineCreateInfo
{
	std::shared_ptr<const ShaderGroup> m_ShaderGroup;
	std::map<uint32_t, std::shared_ptr<DescriptorSetLayout>> m_DescriptorSetLayouts;

	std::optional<vk::VertexInputBindingDescription> m_VertexInputBindingDescription;
	std::vector<vk::VertexInputAttributeDescription> m_VertexInputAttributeDescriptions;
};