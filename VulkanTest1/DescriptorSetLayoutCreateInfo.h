#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>

struct DescriptorSetLayoutCreateInfo
{
	std::vector<vk::DescriptorSetLayoutBinding> m_Bindings;
};