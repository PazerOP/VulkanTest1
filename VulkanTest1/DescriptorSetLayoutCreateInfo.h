#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>

struct DescriptorSetLayoutCreateInfo
{
	std::string m_DebugName;

	std::vector<vk::DescriptorSetLayoutBinding> m_Bindings;
};