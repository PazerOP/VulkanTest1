#include "stdafx.h"
#include "BuiltinUniformBuffers.h"

#include "LogicalDevice.h"

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

BuiltinUniformBuffers::BuiltinUniformBuffers(LogicalDevice& device) :
	m_Device(device)
{
	static_assert(Enums::count<Type>() == std::tuple_size_v<decltype(m_Buffers)>);

	InitBuffers();
	InitDescriptorSetLayout();
	InitDescriptorSet();
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

		m_Buffers[Enums::value_to_index(Type::FrameConstants)]->Write(&frame, sizeof(frame), 0);
	}

	ViewConstants view;
	{
		const auto& swapchainExtent = GetDevice().GetSwapchain().GetInitValues().m_Extent2D;
		const glm::vec2 swapchainHalfSize(swapchainExtent.width / 2.0f, swapchainExtent.height / 2.0f);
		view.orthoProj = glm::ortho<float>(-swapchainHalfSize.x, swapchainHalfSize.x, -swapchainHalfSize.y, swapchainHalfSize.y,
										   -10.0f, 10.0f);

		m_Buffers[Enums::value_to_index(Type::ViewConstants)]->Write(&view, sizeof(view), 0);
	}
}

std::vector<vk::DescriptorSet> BuiltinUniformBuffers::GetDescriptorSets() const
{
	std::vector<vk::DescriptorSet> retVal;

	for (const auto& set : m_DescriptorSets)
		retVal.push_back(set.get());

	return retVal;
}

void BuiltinUniformBuffers::InitBuffers()
{
	const vk::MemoryPropertyFlags flags = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;

	m_Buffers[Enums::value_to_index(Type::FrameConstants)].emplace(m_Device, sizeof(FrameConstants), flags);
	m_Buffers[Enums::value_to_index(Type::ViewConstants)].emplace(m_Device, sizeof(ViewConstants), flags);
}

void BuiltinUniformBuffers::InitDescriptorSetLayout()
{
	vk::DescriptorSetLayoutBinding layoutBindings[] =
	{
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics),
	};

	vk::DescriptorSetLayoutCreateInfo createInfo;
	createInfo.setBindingCount(std::size(layoutBindings));
	createInfo.setPBindings(layoutBindings);

	m_DescriptorSetLayout = GetDevice()->createDescriptorSetLayoutUnique(createInfo);
}

void BuiltinUniformBuffers::InitDescriptorSet()
{
	vk::DescriptorSetLayout layouts[] =
	{
		m_DescriptorSetLayout.get(),
	};

	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.setDescriptorPool(GetDevice().GetDescriptorPool());
	allocInfo.setDescriptorSetCount(std::size(layouts));
	allocInfo.setPSetLayouts(layouts);

	m_DescriptorSets = GetDevice()->allocateDescriptorSetsUnique(allocInfo);

	std::vector<vk::DescriptorBufferInfo> bufferInfos;
	for (auto& buffer : m_Buffers)
	{
		bufferInfos.emplace_back();

		vk::DescriptorBufferInfo& bufferInfo = bufferInfos.back();
		bufferInfo.setBuffer(buffer->GetBuffer());
		bufferInfo.setRange(buffer->GetCreateInfo().size);
	}

	std::array<vk::WriteDescriptorSet, 2> descriptorWrites;
	std::array<vk::CopyDescriptorSet, 0> descriptorCopies;

	descriptorWrites[0].setDstSet(m_DescriptorSets.front().get());
	descriptorWrites[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
	descriptorWrites[0].setDescriptorCount(Enums::count<Type>());
	descriptorWrites[0].setPBufferInfo(&bufferInfos.front());

	/*descriptorWrites[1].setDstSet(m_DescriptorSets.front().get());
	descriptorWrites[1].setDescriptorType(vk::DescriptorType::eUniformBuffer);
	descriptorWrites[1].setDescriptorCount(1);
	descriptorWrites[1].setPBufferInfo(&bufferInfos[1]);*/
	
	GetDevice()->updateDescriptorSets(descriptorWrites, descriptorCopies);
}
