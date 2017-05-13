#include "stdafx.h"
#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(LogicalDevice& device, vk::DeviceSize size, const vk::MemoryPropertyFlags& flags) :
	Buffer(device, size, vk::BufferUsageFlagBits::eUniformBuffer, flags)
{
}
