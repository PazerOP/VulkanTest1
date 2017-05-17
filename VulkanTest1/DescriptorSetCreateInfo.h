#pragma once
#include <map>
#include <memory>
#include <variant>

class Buffer;
class DescriptorSetLayout;
class Texture;

struct DescriptorSetCreateInfo
{
	using DataVariant = std::variant<std::shared_ptr<Buffer>, std::shared_ptr<Texture>>;

	std::map<uint32_t, DataVariant> m_Data;

	std::shared_ptr<const DescriptorSetLayout> m_Layout;
};