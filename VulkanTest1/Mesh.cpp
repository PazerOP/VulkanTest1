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
	const auto& pipeline = GetDevice().GetGraphicsPipeline();

	cmdBuf.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.GetPipeline());

	cmdBuf.bindVertexBuffers(0, GetBuffer().GetBuffer(), vk::DeviceSize(0));
	cmdBuf.bindIndexBuffer(GetBuffer().GetBuffer(), m_VertexList->GetVertexDataSize(), vk::IndexType::eUint32);

	const auto& descriptorSets = pipeline.GetDescriptorSets();
	cmdBuf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.GetPipelineLayout(), 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

	cmdBuf.drawIndexed(m_VertexList->GetIndexCount(), 1, 0, 0, 0);
}

Mesh::Mesh(const std::shared_ptr<const IVertexList>& vertexList, LogicalDevice& device) :
	m_Device(device)
{
	Log::Msg<LogType::ObjectLifetime>(__FUNCSIG__);

	m_Buffer.emplace(device, vertexList->GetVertexDataSize() + vertexList->GetIndexDataSize(),
					 vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
					 vk::MemoryPropertyFlagBits::eDeviceLocal);

	m_VertexList = vertexList;

	Buffer tempBuffer(device, GetBuffer().GetCreateInfo().size,
					  vk::BufferUsageFlagBits::eTransferSrc,
					  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void* data = device->mapMemory(tempBuffer.GetDeviceMemory(), 0, GetBuffer().GetCreateInfo().size);
	memcpy(data, vertexList->GetVertexData(), vertexList->GetVertexDataSize());
	memcpy((char*)data + vertexList->GetVertexDataSize(), vertexList->GetIndexData(), vertexList->GetIndexDataSize());
	device->unmapMemory(tempBuffer.GetDeviceMemory());

	tempBuffer.CopyTo(GetBuffer());
}