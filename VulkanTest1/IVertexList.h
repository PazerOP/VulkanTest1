#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

class IVertexList
{
public:
	virtual ~IVertexList() = default;

	virtual size_t GetVertexSize() const = 0;
	virtual size_t GetVertexCount() const = 0;
	virtual const void* GetVertexData() const = 0;

	virtual vk::VertexInputBindingDescription GetBindingDescription() const = 0;
	virtual std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions() const = 0;
};