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

		m_Buffers[Enums::value_to_index(Binding::FrameConstants)]->Write(&frame, sizeof(frame), 0);
	}

	ViewConstants view;
	{
		const auto& swapchainExtent = GetDevice().GetSwapchain().GetInitValues().m_Extent2D;
		const glm::vec2 swapchainHalfSize(swapchainExtent.width / 2.0f, swapchainExtent.height / 2.0f);

		view.view = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));

		view.orthoProj = glm::ortho<float>(-swapchainHalfSize.x, swapchainHalfSize.x, -swapchainHalfSize.y, swapchainHalfSize.y,
										   0, 10);

		m_Buffers[Enums::value_to_index(Binding::ViewConstants)]->Write(&view, sizeof(view), 0);
	}
}

const std::vector<std::shared_ptr<const DescriptorSet>>& BuiltinUniformBuffers::GetDescriptorSets() const
{
	return reinterpret_cast<const std::vector<std::shared_ptr<const DescriptorSet>>&>(m_DescriptorSets);
}

std::shared_ptr<const DescriptorSetLayout> BuiltinUniformBuffers::GetDescriptorSetLayout(Set set) const
{
	assert(Enums::validate(set));
	return m_DescriptorSetLayouts[Enums::value(set)];
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

		// Frame constants
		binding.setBinding(0);
		createInfo->m_Bindings.push_back(binding);

		// View constants
		binding.setBinding(1);
		createInfo->m_Bindings.push_back(binding);

		m_DescriptorSetLayouts.emplace_back(std::make_shared<DescriptorSetLayout>(m_Device, createInfo));
	}

	// Object constants
	{
		auto createInfo = std::make_shared<DescriptorSetLayoutCreateInfo>();

		binding.setBinding(0);
		createInfo->m_Bindings.push_back(binding);

		m_DescriptorSetLayouts.emplace_back(std::make_shared<DescriptorSetLayout>(m_Device, createInfo));
	}
}

void BuiltinUniformBuffers::InitDescriptorSets()
{
	// Frame/view constants
	{
		auto createInfo = std::make_shared<DescriptorSetCreateInfo>();

		createInfo->m_Data.push_back(m_Buffers[Enums::value(Binding::FrameConstants)]);
		createInfo->m_Data.push_back(m_Buffers[Enums::value(Binding::ViewConstants)]);

		createInfo->m_Layout = m_DescriptorSetLayouts[Enums::value(Set::FrameView)];

		m_DescriptorSets.push_back(std::make_shared<DescriptorSet>(m_Device, createInfo));
	}
}
