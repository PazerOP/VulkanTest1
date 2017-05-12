#pragma once
#include <vulkan/vulkan.hpp>

class IMaterial
{
public:
	virtual ~IMaterial() = default;

	virtual void Bind(const vk::CommandBuffer& cmdBuf) const = 0;
};