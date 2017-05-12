#pragma once
#include <vulkan/vulkan.hpp>

class LogicalDevice;

class Buffer
{
public:
	Buffer(LogicalDevice& device, vk::DeviceSize size, const vk::BufferUsageFlags& bufFlags, const vk::MemoryPropertyFlags& memFlags);
	virtual ~Buffer();

	const LogicalDevice& GetDevice() const { return m_Device; }
	LogicalDevice& GetDevice() { return m_Device; }

	vk::DeviceMemory GetDeviceMemory() const { return m_DeviceMemory.get(); }
	vk::Buffer GetBuffer() const { return m_Buffer.get(); }

	const vk::BufferCreateInfo& GetCreateInfo() const { return m_CreateInfo; }

	void CopyTo(Buffer& buffer) const;

	void Write(const void* data, vk::DeviceSize bytes, vk::DeviceSize offset);

private:
	LogicalDevice& m_Device;
	vk::BufferCreateInfo m_CreateInfo;
	vk::MemoryRequirements m_MemoryReqs;
	vk::MemoryAllocateInfo m_AllocInfo;
	vk::UniqueBuffer m_Buffer;
	vk::UniqueDeviceMemory m_DeviceMemory;
};