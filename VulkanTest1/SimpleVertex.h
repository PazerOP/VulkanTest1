#pragma once
#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

#include <array>
#include <cstddef>

struct SimpleVertex
{
	constexpr SimpleVertex(const glm::vec2& pos, const glm::vec3& color, const glm::vec2& texCoord) :
		m_Pos(pos), m_Color(color), m_TexCoord(texCoord)
	{
	}

	glm::vec2 m_Pos;
	glm::vec3 m_Color;
	glm::vec2 m_TexCoord;

	bool operator==(const SimpleVertex& rhs) const
	{
		return (m_Pos == rhs.m_Pos &&
				m_Color == rhs.m_Color &&
				m_TexCoord == rhs.m_TexCoord);
	}

	static const vk::VertexInputBindingDescription& GetBindingDescription()
	{
		static vk::VertexInputBindingDescription s_BindingDescription(0, sizeof(SimpleVertex), vk::VertexInputRate::eVertex);

		return s_BindingDescription;
	}

	static const std::vector<vk::VertexInputAttributeDescription>& GetAttributeDescriptions()
	{
		static std::vector<vk::VertexInputAttributeDescription> s_AttributeDescriptions(
		{
			vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, offsetof(SimpleVertex, m_Pos)),
			vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(SimpleVertex, m_Color)),
			vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(SimpleVertex, m_TexCoord)),
		});

		return s_AttributeDescriptions;
	}
};