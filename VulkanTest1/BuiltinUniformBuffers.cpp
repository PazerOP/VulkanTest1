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
	float time;
	// Time
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		time = std::chrono::duration<float>(currentTime - startTime).count();

		m_Buffers[Enums::value_to_index(Type::Time)]->Write(&time, sizeof(time), 0);
	}

	time = 0;
	// Transform
	{
		BuiltinTransformBuffer ubo;
		ubo.model = glm::rotate(glm::mat4(), time * glm::radians(90.0f), glm::vec3(0, 0, 1));
		ubo.view = glm::lookAt(glm::vec3(2, 2, 2), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));

		const auto& swapchainExtent = GetDevice().GetSwapchain().GetInitValues().m_Extent2D;
		ubo.proj = glm::perspective(glm::radians(45.0f), float(swapchainExtent.width) / swapchainExtent.height, 0.1f, 10.0f);

		m_Buffers[Enums::value_to_index(Type::Transform)]->Write(&ubo, sizeof(ubo), 0);
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

	m_Buffers[Enums::value_to_index(Type::Transform)].emplace(m_Device, sizeof(BuiltinTransformBuffer), flags);

	m_Buffers[Enums::value_to_index(Type::Time)].emplace(m_Device, sizeof(BuiltinTimeBuffer), flags);
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

	std::array<vk::WriteDescriptorSet, 1> descriptorWrites;
	std::array<vk::CopyDescriptorSet, 0> descriptorCopies;

	descriptorWrites[0].setDstSet(m_DescriptorSets.front().get());
	descriptorWrites[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
	descriptorWrites[0].setDescriptorCount(2);
	descriptorWrites[0].setPBufferInfo(&bufferInfos[0]);

	/*descriptorWrites[1].setDstSet(m_DescriptorSets.front().get());
	descriptorWrites[1].setDescriptorType(vk::DescriptorType::eUniformBuffer);
	descriptorWrites[1].setDescriptorCount(1);
	descriptorWrites[1].setPBufferInfo(&bufferInfos[1]);*/
	
	GetDevice()->updateDescriptorSets(descriptorWrites, descriptorCopies);
}
