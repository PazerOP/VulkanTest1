#pragma once
#include <memory>
#include <optional>
#include <variant>
#include <vector>

class Buffer;
class DescriptorSetLayout;
class Texture;

struct DescriptorSetCreateInfo
{
	using DataVariant = std::variant<
		std::shared_ptr<Buffer>,
		std::shared_ptr<Texture>
	>;

	struct Binding
	{
		Binding() = default;
		Binding(uint32_t bindingIndex, const vk::ShaderStageFlags& stages,
				const DataVariant& data, const std::string& debugName = std::string()) :
			m_BindingIndex(bindingIndex),
			m_Stages(stages),
			m_Data(data),
			m_DebugName(debugName)
		{
		}

		std::string m_DebugName;
		vk::ShaderStageFlags m_Stages;
		std::optional<uint32_t> m_BindingIndex;
		DataVariant m_Data;
	};

	std::vector<Binding> m_Data;

	std::shared_ptr<const DescriptorSetLayout> m_Layout;
};