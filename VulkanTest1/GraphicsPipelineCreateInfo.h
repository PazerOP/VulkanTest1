#pragma once
#include "ShaderType.h"

#include <vulkan/vulkan.hpp>

#include <map>
#include <optional>
#include <variant>

class DescriptorSetLayout;
class ShaderGroup;

struct GraphicsPipelineCreateInfo
{
	using SpecializationVariant = std::variant<bool, int64_t, uint64_t, double>;
	using Specializations = std::map<ShaderType, std::map<uint32_t, SpecializationVariant>>;

	std::shared_ptr<const ShaderGroup> m_ShaderGroup;
	std::map<uint32_t, std::shared_ptr<DescriptorSetLayout>> m_DescriptorSetLayouts;

	std::optional<vk::VertexInputBindingDescription> m_VertexInputBindingDescription;
	std::vector<vk::VertexInputAttributeDescription> m_VertexInputAttributeDescriptions;

	Specializations m_Specializations;
};