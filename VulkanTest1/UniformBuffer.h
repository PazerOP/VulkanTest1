#pragma once
#include "Buffer.h"

class UniformBuffer : public Buffer
{
public:
	UniformBuffer(LogicalDevice& device, vk::DeviceSize size, const vk::MemoryPropertyFlags& flags);
};