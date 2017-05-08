#pragma once
#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

#include <array>
#include <cstddef>

struct SimpleVertex
{
	glm::vec2 pos;
	glm::vec3 color;

	bool operator==(const SimpleVertex& rhs) const
	{
		return (pos == rhs.pos && color == rhs.color);
	}

	static vk::VertexInputBindingDescription GetBindingDescription()
	{
		vk::VertexInputBindingDescription desc;
		desc.setBinding(0);
		desc.setStride(sizeof(SimpleVertex));
		desc.setInputRate(vk::VertexInputRate::eVertex);

		return desc;
	}

	static std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions()
	{
		std::vector<vk::VertexInputAttributeDescription> retVal(2);

		retVal[0].setBinding(0);
		retVal[0].setLocation(0);
		retVal[0].setFormat(vk::Format::eR32G32Sfloat);
		retVal[0].setOffset(offsetof(SimpleVertex, pos));

		retVal[1].setBinding(0);
		retVal[1].setLocation(1);
		retVal[1].setFormat(vk::Format::eR32G32B32Sfloat);
		retVal[1].setOffset(offsetof(SimpleVertex, color));

		return retVal;
	}
};