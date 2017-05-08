#pragma once
#include "Buffer.h"

class IVertexList;
class LogicalDevice;

class VertexBuffer : public Buffer
{
public:
	static std::unique_ptr<VertexBuffer> Create(const std::shared_ptr<const IVertexList>& vertexList);
	static std::unique_ptr<VertexBuffer> Create(const std::shared_ptr<const IVertexList>& vertexList, LogicalDevice& device);

	void Draw(const vk::CommandBuffer& buffer) const;

private:
	VertexBuffer(const std::shared_ptr<const IVertexList>& vertexList, LogicalDevice& device);

	std::shared_ptr<const IVertexList> m_VertexList;
};