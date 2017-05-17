#pragma once
#include <memory>
#include <variant>
#include <vector>

class Buffer;
class DescriptorSetLayout;
class Texture;

struct DescriptorSetCreateInfo
{
	using DataVariant = std::variant<std::shared_ptr<Buffer>, std::shared_ptr<Texture>>;

	std::vector<DataVariant> m_Data;

	std::shared_ptr<const DescriptorSetLayout> m_Layout;
};