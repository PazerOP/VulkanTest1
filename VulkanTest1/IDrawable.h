#pragma once
#include <vulkan/vulkan.hpp>

class IDrawable
{
public:
	virtual ~IDrawable() = default;

	virtual void Draw(const vk::CommandBuffer& cmdBuf) const = 0;
};