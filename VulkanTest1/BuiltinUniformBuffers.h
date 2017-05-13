#pragma once
#include "Enums.h"
#include "UniformBuffer.h"
#include "Util.h"

#include <optional>

struct BuiltinTransformBuffer
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

struct BuiltinTimeBuffer
{
	float time;
};

class BuiltinUniformBuffers
{
public:
	BuiltinUniformBuffers(LogicalDevice& device);

	LogicalDevice& GetDevice() const { return m_Device; }

	void Update();

	enum class Type
	{
		Transform,
		Time,
	};

	static constexpr auto Names = make_array<const char*>(
		"__transform",
		"__time"
	);

	vk::DescriptorSetLayout GetDescriptorSetLayout() const { return m_DescriptorSetLayout.get(); }
	std::vector<vk::DescriptorSet> GetDescriptorSets() const;

private:
	LogicalDevice& m_Device;

	void InitBuffers();
	void InitDescriptorSetLayout();
	void InitDescriptorSet();

	static constexpr auto BUFFER_COUNT = 2;
	std::array<std::optional<UniformBuffer>, BUFFER_COUNT> m_Buffers;

	vk::UniqueDescriptorSetLayout m_DescriptorSetLayout;
	std::vector<vk::UniqueDescriptorSet> m_DescriptorSets;
};

template<> __forceinline constexpr auto Enums::min<BuiltinUniformBuffers::Type>() { return Enums::value(BuiltinUniformBuffers::Type::Transform); }
template<> __forceinline constexpr auto Enums::max<BuiltinUniformBuffers::Type>() { return Enums::value(BuiltinUniformBuffers::Type::Time); }