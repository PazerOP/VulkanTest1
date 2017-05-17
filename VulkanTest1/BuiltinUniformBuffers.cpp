#include "stdafx.h"
#include "BuiltinUniformBuffers.h"

#include "DescriptorSet.h"
#include "DescriptorSetCreateInfo.h"
#include "DescriptorSetLayout.h"
#include "DescriptorSetLayoutCreateInfo.h"
#include "LogicalDevice.h"

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

BuiltinUniformBuffers::BuiltinUniformBuffers(LogicalDevice& device) :
	m_Device(device)
{
	InitBuffers();
	InitDescriptorSetLayouts();
	InitDescriptorSets();
}

void BuiltinUniformBuffers::Update()
{
	FrameConstants frame;
	{
		static auto startTime = std::chrono::high_resolution_clock::now();
		static auto lastTime = startTime;

		auto currentTime = std::chrono::high_resolution_clock::now();
		auto test = currentTime - startTime;
		frame.time = std::chrono::duration<float>(currentTime - startTime).count();
		frame.dt = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;

		m_Buffers[Enums::value_to_index(FrameViewBindings::Frame)]->Write(&frame, sizeof(frame), 0);
	}

	ViewConstants view;
	{
		const auto& swapchainExtent = GetDevice().GetSwapchain().GetInitValues().m_Extent2D;
		const glm::vec2 swapchainHalfSize(swapchainExtent.width / 2.0f, swapchainExtent.height / 2.0f);

		view.view = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));

		view.orthoProj = glm::ortho<float>(-swapchainHalfSize.x, swapchainHalfSize.x, -swapchainHalfSize.y, swapchainHalfSize.y,
										   0, 10);

		m_Buffers[Enums::value_to_index(FrameViewBindings::View)]->Write(&view, sizeof(view), 0);
	}
}

const std::map<BuiltinUniformBuffers::Set, std::shared_ptr<const DescriptorSet>>& BuiltinUniformBuffers::GetDescriptorSets() const
{
	return reinterpret_cast<const std::map<Set, std::shared_ptr<const DescriptorSet>>&>(m_DescriptorSets);
}

const std::map<BuiltinUniformBuffers::Set, std::shared_ptr<const DescriptorSetLayout>>& BuiltinUniformBuffers::GetDescriptorSetLayouts() const
{
	return reinterpret_cast<const std::map<BuiltinUniformBuffers::Set, std::shared_ptr<const DescriptorSetLayout>>&>(m_DescriptorSetLayouts);
}

const std::map<uint32_t, std::shared_ptr<const DescriptorSetLayout>>& BuiltinUniformBuffers::GetDescriptorSetLayoutsUint() const
{
	return reinterpret_cast<const std::map<uint32_t, std::shared_ptr<const DescriptorSetLayout>>&>(GetDescriptorSetLayouts());
}

const std::map<uint32_t, std::shared_ptr<DescriptorSetLayout>>& BuiltinUniformBuffers::GetDescriptorSetLayoutsUint()
{
	static_assert(std::is_same_v<std::underlying_type_t<Set>, uint32_t>);

	return reinterpret_cast<const std::map<uint32_t, std::shared_ptr<DescriptorSetLayout>>&>(GetDescriptorSetLayouts());
}

std::shared_ptr<const DescriptorSetLayout> BuiltinUniformBuffers::GetDescriptorSetLayout(Set set) const
{
	assert(Enums::validate(set));
	return m_DescriptorSetLayouts.at(set);
}

void BuiltinUniformBuffers::InitBuffers()
{
	const vk::MemoryPropertyFlags flags = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;

	m_Buffers.push_back(std::make_shared<UniformBuffer>(m_Device, sizeof(FrameConstants), flags));
	m_Buffers.push_back(std::make_shared<UniformBuffer>(m_Device, sizeof(ViewConstants), flags));
}

void BuiltinUniformBuffers::InitDescriptorSetLayouts()
{
	m_DescriptorSetLayouts.clear();

	// They're mostly all the same (for now?)
	vk::DescriptorSetLayoutBinding binding;
	{
		binding.setDescriptorCount(1);
		binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
		binding.setStageFlags(vk::ShaderStageFlagBits::eAll);
	}

	// Frame/view constants
	{
		auto createInfo = std::make_shared<DescriptorSetLayoutCreateInfo>();
		createInfo->m_DebugName = __FUNCSIG__ ": Frame/View constants";

		// Frame constants
		binding.setBinding(Enums::value(FrameViewBindings::Frame));
		createInfo->m_Bindings.push_back(binding);

		// View constants
		binding.setBinding(Enums::value(FrameViewBindings::View));
		createInfo->m_Bindings.push_back(binding);

		m_DescriptorSetLayouts.insert(std::make_pair(Set::FrameView, std::make_shared<DescriptorSetLayout>(m_Device, createInfo)));
	}

	// Object constants
	{
		auto createInfo = std::make_shared<DescriptorSetLayoutCreateInfo>();
		createInfo->m_DebugName = __FUNCSIG__ ": Object constants";

		binding.setBinding(0);
		createInfo->m_Bindings.push_back(binding);

		m_DescriptorSetLayouts.insert(std::make_pair(Set::Object, std::make_shared<DescriptorSetLayout>(m_Device, createInfo)));
	}
}

void BuiltinUniformBuffers::InitDescriptorSets()
{
	// Frame/view constants
	{
		auto createInfo = std::make_shared<DescriptorSetCreateInfo>();

		createInfo->m_Data.push_back(DescriptorSetCreateInfo::Binding(Enums::value(FrameViewBindings::Frame), vk::ShaderStageFlagBits::eAll, m_Buffers[Enums::value(FrameViewBindings::Frame)], "FrameViewBindings::Frame"));

		createInfo->m_Data.push_back(DescriptorSetCreateInfo::Binding(Enums::value(FrameViewBindings::View), vk::ShaderStageFlagBits::eAll, m_Buffers[Enums::value(FrameViewBindings::View)], "FrameViewBindings::View"));

		createInfo->m_Layout = m_DescriptorSetLayouts.at(Set::FrameView);

		m_DescriptorSets.insert(std::make_pair(Set::FrameView, std::make_shared<DescriptorSet>(m_Device, createInfo)));
	}
}
