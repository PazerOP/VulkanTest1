#pragma once
#include "Enums.h"
#include "UniformBuffer.h"
#include "Util.h"

#include <optional>

class DescriptorSet;
class DescriptorSetLayout;

class BuiltinUniformBuffers
{
public:
	BuiltinUniformBuffers(LogicalDevice& device);

	LogicalDevice& GetDevice() const { return m_Device; }

	void Update();

	enum class Binding
	{
		FrameConstants,
		ViewConstants,
		ObjectConstants,
	};
	enum class Set
	{
		FrameView,
		Object,
	};

	struct FrameConstants
	{
		float time;
		float dt;
	};

	struct ViewConstants
	{
		glm::vec2 camPos;
		alignas(16) glm::mat4 view;			// "view" in the MVP matrix set
		alignas(16) glm::mat4 orthoProj;	// "model" in MVP matrix set
	};

	const std::vector<std::shared_ptr<const DescriptorSet>>& GetDescriptorSets() const;
	const std::vector<std::shared_ptr<DescriptorSet>>& GetDescriptorSets() { return m_DescriptorSets; }

	const std::vector<std::shared_ptr<const DescriptorSetLayout>>& GetDescriptorSetLayouts() const;
	const std::vector<std::shared_ptr<DescriptorSetLayout>>& GetDescriptorSetLayouts() { return m_DescriptorSetLayouts; }

	std::shared_ptr<const DescriptorSetLayout> GetDescriptorSetLayout(Set set) const;

private:

	LogicalDevice& m_Device;

	void InitBuffers();
	void InitDescriptorSetLayouts();
	void InitDescriptorSets();

	std::vector<std::shared_ptr<Buffer>> m_Buffers;
	std::vector<std::shared_ptr<DescriptorSet>> m_DescriptorSets;
	std::vector<std::shared_ptr<DescriptorSetLayout>> m_DescriptorSetLayouts;
};

template<> __forceinline constexpr auto Enums::min<BuiltinUniformBuffers::Binding>() { return Enums::value(BuiltinUniformBuffers::Binding::FrameConstants); }
template<> __forceinline constexpr auto Enums::max<BuiltinUniformBuffers::Binding>() { return Enums::value(BuiltinUniformBuffers::Binding::ObjectConstants); }
template<> __forceinline constexpr auto Enums::min<BuiltinUniformBuffers::Set>() { return Enums::value(BuiltinUniformBuffers::Set::FrameView); }
template<> __forceinline constexpr auto Enums::max<BuiltinUniformBuffers::Set>() { return Enums::value(BuiltinUniformBuffers::Set::Object); }