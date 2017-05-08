#pragma once
#include "Buffer.h"

class IVertexList;
class LogicalDevice;

class Mesh
{
public:
	static std::unique_ptr<Mesh> Create(const std::shared_ptr<const IVertexList>& vertexList);
	static std::unique_ptr<Mesh> Create(const std::shared_ptr<const IVertexList>& vertexList, LogicalDevice& device);

	void Draw(const vk::CommandBuffer& buffer) const;

	const LogicalDevice& GetDevice() const { return m_VertexBuffer.GetDevice(); }
	LogicalDevice& GetDevice() { return m_VertexBuffer.GetDevice(); }

	const Buffer& GetVertexBuffer() const { return m_VertexBuffer; }
	Buffer& GetVertexBuffer() { return m_VertexBuffer; }

	const Buffer& GetIndexBuffer() const { return m_IndexBuffer; }
	Buffer& GetIndexBuffer() { return m_IndexBuffer; }

private:
	Mesh(const std::shared_ptr<const IVertexList>& vertexList, LogicalDevice& device);

	std::shared_ptr<const IVertexList> m_VertexList;
	Buffer m_VertexBuffer;
	Buffer m_IndexBuffer;
};