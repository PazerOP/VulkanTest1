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

	const LogicalDevice& GetDevice() const { return m_Buffer->GetDevice(); }
	LogicalDevice& GetDevice() { return m_Buffer->GetDevice(); }

	const Buffer& GetBuffer() const { return m_Buffer.value(); }
	Buffer& GetBuffer() { return m_Buffer.value(); }

private:
	Mesh(const std::shared_ptr<const IVertexList>& vertexList, LogicalDevice& device);

	LogicalDevice& m_Device;

	std::shared_ptr<const IVertexList> m_VertexList;
	std::optional<Buffer> m_Buffer;
};