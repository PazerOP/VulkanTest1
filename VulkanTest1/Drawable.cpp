#include "stdafx.h"
#include "Drawable.h"

Drawable::Drawable(LogicalDevice& device) :
	m_Device(device),
	m_ObjectConstantsBuffer(m_Device, sizeof(ObjectConstants), vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
{
}

void Drawable::Update()
{
	ObjectConstants obj;
	obj.modelToWorld = m_Transform.ComputeMatrix();
	m_ObjectConstantsBuffer.Write(&obj, sizeof(obj), 0);
}

void Drawable::Draw(const vk::CommandBuffer& cmdBuf) const
{
	GetMaterial().Bind(cmdBuf);
	GetMesh().Draw(cmdBuf);
}
