#pragma once
#include "Enums.h"
#include "UniformBuffer.h"
#include "Util.h"

#include <optional>

class BuiltinUniformBuffers
{
public:
	BuiltinUniformBuffers(LogicalDevice& device);

	LogicalDevice& GetDevice() const { return m_Device; }

	void Update();

	enum class Type
	{
		FrameConstants,
		ViewConstants,
		ObjectConstants,
	};

	struct FrameConstants
	{
		float time;
		float dt;
	};

	struct ViewConstants
	{
		glm::vec3 camPos;
	};

	struct ObjectConstants
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	vk::DescriptorSetLayout GetDescriptorSetLayout() const { return m_DescriptorSetLayout.get(); }
	std::vector<vk::DescriptorSet> GetDescriptorSets() const;

private:
	LogicalDevice& m_Device;

	void InitBuffers();
	void InitDescriptorSetLayout();
	void InitDescriptorSet();

	static constexpr auto BUFFER_COUNT = 3;
	std::array<std::optional<UniformBuffer>, BUFFER_COUNT> m_Buffers;

	vk::UniqueDescriptorSetLayout m_DescriptorSetLayout;
	std::vector<vk::UniqueDescriptorSet> m_DescriptorSets;
};

template<> __forceinline constexpr auto Enums::min<BuiltinUniformBuffers::Type>() { return Enums::value(BuiltinUniformBuffers::Type::FrameConstants); }
template<> __forceinline constexpr auto Enums::max<BuiltinUniformBuffers::Type>() { return Enums::value(BuiltinUniformBuffers::Type::ObjectConstants); }