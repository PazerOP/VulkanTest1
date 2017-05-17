#include "stdafx.h"
#include "Buffer.h"

#include "LogicalDevice.h"
#include "VulkanHelpers.h"

Buffer::Buffer(LogicalDevice& device, vk::DeviceSize size, const vk::BufferUsageFlags& bufFlags, const vk::MemoryPropertyFlags& memFlags) :
	m_Device(device)
{
	Log::Msg<LogType::ObjectLifetime>(__FUNCSIG__);

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

	cmdBuf->begin(VulkanHelpers::CBBI_ONE_TIME_SUBMIT);
	cmdBuf->copyBuffer(m_Buffer.get(), buffer.Get(), vk::BufferCopy(0, 0, GetCreateInfo().size));
	cmdBuf->end();

	GetDevice().SubmitCommandBuffers(cmdBuf.get());
}

void Buffer::Write(const void* data, vk::DeviceSize bytes, vk::DeviceSize offset)
{
	void* dst = GetDevice()->mapMemory(m_DeviceMemory.get(), offset, bytes);
	memcpy(dst, data, bytes);
	GetDevice()->unmapMemory(m_DeviceMemory.get());
}