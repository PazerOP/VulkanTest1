#include "stdafx.h"
#include "GraphicsPipeline.h"
#include "IVertexList.h"
#include "LogicalDevice.h"
#include "VertexBuffer.h"
#include "Vulkan.h"

std::unique_ptr<VertexBuffer> VertexBuffer::Create(const std::shared_ptr<const IVertexList>& vertexList)
{
	return std::unique_ptr<VertexBuffer>(new VertexBuffer(vertexList, Vulkan().GetLogicalDevice()));
}

std::unique_ptr<VertexBuffer> VertexBuffer::Create(const std::shared_ptr<const IVertexList>& vertexList, LogicalDevice& device)
{
	return std::unique_ptr<VertexBuffer>(new VertexBuffer(vertexList, device));
}

void VertexBuffer::Draw(const vk::CommandBuffer& cmdBuf) const
{
	cmdBuf.bindPipeline(vk::PipelineBindPoint::eGraphics, GetDevice().GetGraphicsPipeline().GetPipeline());

	auto buffers = make_array<vk::Buffer>(GetBuffer());
	auto offsets = make_array<vk::DeviceSize>((vk::DeviceSize)0);
	cmdBuf.bindVertexBuffers(0, buffers, offsets);

	cmdBuf.draw(m_VertexList->GetVertexCount(), 1, 0, 0);
}

VertexBuffer::VertexBuffer(const std::shared_ptr<const IVertexList>& vertexList, LogicalDevice& device) :
	Buffer(device, vertexList->GetVertexSize() * vertexList->GetVertexCount(),
		   vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		   vk::MemoryPropertyFlagBits::eDeviceLocal)
{
	m_VertexList = vertexList;

	const size_t size = GetCreateInfo().size;

	Buffer tempBuffer(device, size,
					  vk::BufferUsageFlagBits::eTransferSrc,
					  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void* data = device->mapMemory(tempBuffer.GetDeviceMemory(), 0, size);
	memcpy(data, m_VertexList->GetVertexData(), size);
	device->unmapMemory(tempBuffer.GetDeviceMemory());

	tempBuffer.CopyTo(*this);
}