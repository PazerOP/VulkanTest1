#include "stdafx.h"
#include "Drawable.h"

Drawable::Drawable(LogicalDevice& device) :
	m_Device(device)
{
}

void Drawable::Draw(const vk::CommandBuffer& cmdBuf) const
{
	GetMaterial().Bind(cmdBuf);
	GetMesh().Draw(cmdBuf);
}
