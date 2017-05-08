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

	cmdBuf.bindVertexBuffers(0, GetBuffer().GetBuffer(), vk::DeviceSize(0));
	cmdBuf.bindIndexBuffer(GetBuffer().GetBuffer(), m_VertexList->GetVertexDataSize(), vk::IndexType::eUint32);

	cmdBuf.drawIndexed(m_VertexList->GetIndexCount(), 1, 0, 0, 0);
}

Mesh::Mesh(const std::shared_ptr<const IVertexList>& vertexList, LogicalDevice& device) :
	m_Buffer(device, vertexList->GetVertexDataSize() + vertexList->GetIndexDataSize(),
			 vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
			 vk::MemoryPropertyFlagBits::eDeviceLocal)
{
	m_VertexList = vertexList;

	Buffer tempBuffer(device, m_Buffer.GetCreateInfo().size,
					  vk::BufferUsageFlagBits::eTransferSrc,
					  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void* data = device->mapMemory(tempBuffer.GetDeviceMemory(), 0, m_Buffer.GetCreateInfo().size);
	memcpy(data, vertexList->GetVertexData(), vertexList->GetVertexDataSize());
	memcpy((char*)data + vertexList->GetVertexDataSize(), vertexList->GetIndexData(), vertexList->GetIndexDataSize());
	device->unmapMemory(tempBuffer.GetDeviceMemory());

	tempBuffer.CopyTo(m_Buffer);
}