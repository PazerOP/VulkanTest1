#pragma once
#include "Enums.h"
#include "UniformBuffer.h"
#include "Util.h"

#include <map>
#include <optional>

class DescriptorSet;
class DescriptorSetLayout;

class BuiltinUniformBuffers
{
public:
	BuiltinUniformBuffers(LogicalDevice& device);

	LogicalDevice& GetDevice() const { return m_Device; }

	void Update();

	enum class FrameViewBindings : uint32_t
	{
		Frame,
		View,
	};
	enum class Set : uint32_t
	{
		FrameView,
		Material,
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

	const std::map<Set, std::shared_ptr<const DescriptorSet>>& GetDescriptorSets() const;
	const auto& GetDescriptorSets() { return m_DescriptorSets; }

	const std::map<Set, std::shared_ptr<const DescriptorSetLayout>>& GetDescriptorSetLayouts() const;
	const auto& GetDescriptorSetLayouts() { return m_DescriptorSetLayouts; }

	const std::map<uint32_t, std::shared_ptr<const DescriptorSetLayout>>& GetDescriptorSetLayoutsUint() const;
	const std::map<uint32_t, std::shared_ptr<DescriptorSetLayout>>& GetDescriptorSetLayoutsUint();

	std::shared_ptr<const DescriptorSetLayout> GetDescriptorSetLayout(Set set) const;

private:

	LogicalDevice& m_Device;

	void InitBuffers();
	void InitDescriptorSetLayouts();
	void InitDescriptorSets();

	std::vector<std::shared_ptr<Buffer>> m_Buffers;
	std::map<Set, std::shared_ptr<DescriptorSet>> m_DescriptorSets;
	std::map<Set, std::shared_ptr<DescriptorSetLayout>> m_DescriptorSetLayouts;
};

template<> __forceinline constexpr auto Enums::min<BuiltinUniformBuffers::FrameViewBindings>() { return Enums::value(BuiltinUniformBuffers::FrameViewBindings::Frame); }
template<> __forceinline constexpr auto Enums::max<BuiltinUniformBuffers::FrameViewBindings>() { return Enums::value(BuiltinUniformBuffers::FrameViewBindings::View); }
template<> __forceinline constexpr auto Enums::min<BuiltinUniformBuffers::Set>() { return Enums::value(BuiltinUniformBuffers::Set::FrameView); }
template<> __forceinline constexpr auto Enums::max<BuiltinUniformBuffers::Set>() { return Enums::value(BuiltinUniformBuffers::Set::Object); }