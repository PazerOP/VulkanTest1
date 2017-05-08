#pragma once

class IVertexList;
class LogicalDevice;

class VertexBuffer
{
public:
	static std::unique_ptr<VertexBuffer> Create(const std::shared_ptr<const IVertexList>& vertexList);
	static std::unique_ptr<VertexBuffer> Create(const std::shared_ptr<const IVertexList>& vertexList, const LogicalDevice& device);

	const vk::BufferCreateInfo& GetCreateInfo() const { return m_CreateInfo; }

	vk::Buffer GetBuffer() const { return m_Buffer.get(); }

	void Draw(const vk::CommandBuffer& buffer) const;

private:
	VertexBuffer(const std::shared_ptr<const IVertexList>& vertexList, const LogicalDevice& device);

	mutable bool m_Bound;
	std::shared_ptr<const IVertexList> m_VertexList;
	const LogicalDevice& m_Device;
	vk::BufferCreateInfo m_CreateInfo;
	vk::UniqueBuffer m_Buffer;
	vk::UniqueDeviceMemory m_DeviceMemory;
};