#include "stdafx.h"
#include "GraphicsPipeline.h"
#include "IVertexList.h"
#include "LogicalDevice.h"
#include "VertexBuffer.h"
#include "Vulkan.h"

std::unique_ptr<VertexBuffer> VertexBuffer::Create(const std::shared_ptr<const IVertexList>& vertexList)
{
	return std::unique_ptr<VertexBuffer>(new VertexBuffer(vertexList, Vulkan().GetLogicalDevice()));
}

std::unique_ptr<VertexBuffer> VertexBuffer::Create(const std::shared_ptr<const IVertexList>& vertexList, const LogicalDevice& device)
{
	return std::unique_ptr<VertexBuffer>(new VertexBuffer(vertexList, device));
}

static uint32_t FindMemoryType(const vk::PhysicalDeviceMemoryProperties& memProps, uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}

	throw rkrp_vulkan_exception("failed to find suitable memory type!");
}

void VertexBuffer::Draw(const vk::CommandBuffer& buffer) const
{
	if (!m_Bound)
	{
		m_Device->bindBufferMemory(m_Buffer.get(), m_DeviceMemory.get(), 0);

		void* data = m_Device->mapMemory(m_DeviceMemory.get(), 0, m_CreateInfo.size);
		memcpy(data, m_VertexList->GetVertexData(), m_CreateInfo.size);
		m_Device->unmapMemory(m_DeviceMemory.get());

		m_Bound = true;
	}

	buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Device.GetGraphicsPipeline().GetPipeline());

	auto buffers = make_array<vk::Buffer>(m_Buffer.get());
	auto offsets = make_array<vk::DeviceSize>((vk::DeviceSize)0);
	buffer.bindVertexBuffers(0, buffers, offsets);

	buffer.draw(m_VertexList->GetVertexCount(), 1, 0, 0);
}

VertexBuffer::VertexBuffer(const std::shared_ptr<const IVertexList>& vertexList, const LogicalDevice& device) : m_Device(device)
{
	m_Bound = false;
	m_VertexList = vertexList;

	m_CreateInfo.setSize(vertexList->GetVertexSize() * vertexList->GetVertexCount());
	m_CreateInfo.setUsage(vk::BufferUsageFlagBits::eVertexBuffer);
	m_CreateInfo.setSharingMode(vk::SharingMode::eExclusive);

	m_Buffer = device->createBufferUnique(m_CreateInfo);

	const vk::MemoryRequirements& memReqs = device->getBufferMemoryRequirements(m_Buffer.get());
	assert(memReqs.size > 0);

	const vk::PhysicalDeviceMemoryProperties& memProps = device.GetData().GetMemoryProperties();

	vk::MemoryAllocateInfo allocInfo;
	allocInfo.setAllocationSize(memReqs.size);
	allocInfo.setMemoryTypeIndex(FindMemoryType(memProps, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));

	m_DeviceMemory = device->allocateMemoryUnique(allocInfo);
}
