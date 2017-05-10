#include "stdafx.h"
#include "Buffer.h"

#include "LogicalDevice.h"

Buffer::Buffer(LogicalDevice& device, vk::DeviceSize size, const vk::BufferUsageFlags& bufFlags, const vk::MemoryPropertyFlags& memFlags) :
	m_Device(device)
{
	m_CreateInfo.setSize(size);
	m_CreateInfo.setUsage(bufFlags);
	m_CreateInfo.setSharingMode(vk::SharingMode::eExclusive);

	m_Buffer = device->createBufferUnique(m_CreateInfo);

	m_MemoryReqs = device->getBufferMemoryRequirements(m_Buffer.get());

	m_AllocInfo.setAllocationSize(m_MemoryReqs.size);
	m_AllocInfo.setMemoryTypeIndex(device.GetData().FindMemoryType(m_MemoryReqs.memoryTypeBits, memFlags));

	m_DeviceMemory = device->allocateMemoryUnique(m_AllocInfo);

	device->bindBufferMemory(m_Buffer.get(), m_DeviceMemory.get(), 0);
}

Buffer::~Buffer()
{
	m_Buffer.reset();
	m_DeviceMemory.reset();
}

void Buffer::CopyTo(Buffer& buffer) const
{
	vk::UniqueCommandBuffer cmdBuf = GetDevice().AllocCommandBuffer();

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	cmdBuf->begin(beginInfo);

	vk::BufferCopy copyRegion;
	copyRegion.setSize(GetCreateInfo().size);

	cmdBuf->copyBuffer(m_Buffer.get(), buffer.GetBuffer(), copyRegion);

	cmdBuf->end();

	vk::SubmitInfo submitInfo;

	const vk::CommandBuffer cmdBuffers[] = { cmdBuf.get() };
	submitInfo.setCommandBufferCount(std::size(cmdBuffers));
	submitInfo.setPCommandBuffers(cmdBuffers);

	const auto queue = GetDevice().GetQueue(QueueType::Graphics);
	queue.submit(submitInfo, nullptr);
	queue.waitIdle();
}

void Buffer::Write(const void* data, vk::DeviceSize bytes, vk::DeviceSize offset)
{
	void* dst = GetDevice()->mapMemory(m_DeviceMemory.get(), offset, bytes);
	memcpy(dst, data, bytes);
	GetDevice()->unmapMemory(m_DeviceMemory.get());
}