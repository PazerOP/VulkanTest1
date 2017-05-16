#pragma once
#include <variant>
#include <vector>
#include <vulkan/vulkan.hpp>

class Buffer;
class DescriptorSetLayout;

struct DescriptorSetCreateInfo
{
	using DataVariant = std::variant<std::shared_ptr<Buffer>>;

	std::vector<DataVariant> m_Data;

	std::shared_ptr<const DescriptorSetLayout> m_Layout;
};