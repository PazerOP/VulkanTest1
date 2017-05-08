#include "stdafx.h"
#include "GraphicsPipeline.h"
#include "IVertexList.h"
#include "LogicalDevice.h"
#include "Mesh.h"
#include "Vulkan.h"

std::unique_ptr<Mesh> Mesh::Create(const std::shared_ptr<const IVertexList>& vertexList)
{
	return std::unique_ptr<Mesh>(new Mesh(vertexList, Vulkan().GetLogicalDevice()));
}

std::unique_ptr<Mesh> Mesh::Create(const std::shared_ptr<const IVertexList>& vertexList, LogicalDevice& device)
{
	return std::unique_ptr<Mesh>(new Mesh(vertexList, device));
}

void Mesh::Draw(const vk::CommandBuffer& cmdBuf) const
{
	cmdBuf.bindPipeline(vk::PipelineBindPoint::eGraphics, GetDevice().GetGraphicsPipeline().GetPipeline());

	cmdBuf.bindVertexBuffers(0, GetVertexBuffer().GetBuffer(), vk::DeviceSize(0));
	cmdBuf.bindIndexBuffer(GetIndexBuffer().GetBuffer(), 0, vk::IndexType::eUint32);

	cmdBuf.drawIndexed(m_VertexList->GetIndexCount(), 1, 0, 0, 0);
}

Mesh::Mesh(const std::shared_ptr<const IVertexList>& vertexList, LogicalDevice& device) :
	m_VertexBuffer(device, vertexList->GetVertexSize() * vertexList->GetVertexCount(),
				   vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
				   vk::MemoryPropertyFlagBits::eDeviceLocal),
	m_IndexBuffer(device, vertexList->GetIndexSize() * vertexList->GetIndexCount(),
				  vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
				  vk::MemoryPropertyFlagBits::eDeviceLocal)
{
	m_VertexList = vertexList;

	Buffer tempVertexBuffer(device, m_VertexBuffer.GetCreateInfo().size,
							vk::BufferUsageFlagBits::eTransferSrc,
							vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void* data = device->mapMemory(tempVertexBuffer.GetDeviceMemory(), 0, m_VertexBuffer.GetCreateInfo().size);
	memcpy(data, m_VertexList->GetVertexData(), m_VertexBuffer.GetCreateInfo().size);
	device->unmapMemory(tempVertexBuffer.GetDeviceMemory());

	Buffer tempIndexBuffer(device, m_IndexBuffer.GetCreateInfo().size,
						   vk::BufferUsageFlagBits::eTransferSrc,
						   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	data = device->mapMemory(tempIndexBuffer.GetDeviceMemory(), 0, m_IndexBuffer.GetCreateInfo().size);
	memcpy(data, m_VertexList->GetIndexData(), m_IndexBuffer.GetCreateInfo().size);
	device->unmapMemory(tempIndexBuffer.GetDeviceMemory());

	tempVertexBuffer.CopyTo(m_VertexBuffer);
	tempIndexBuffer.CopyTo(m_IndexBuffer);
}