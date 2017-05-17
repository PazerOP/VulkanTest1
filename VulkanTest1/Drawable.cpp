#include "stdafx.h"
#include "Drawable.h"

#include "BuiltinUniformBuffers.h"
#include "DescriptorSet.h"
#include "DescriptorSetCreateInfo.h"
#include "LogicalDevice.h"

Drawable::Drawable(LogicalDevice& device) :
	m_Device(device)
{
	CreateObjectConstants();
}

void Drawable::Update()
{
	ObjectConstants obj;
	obj.modelToWorld = m_Transform.ComputeMatrix();
	m_ObjectConstantsBuffer->Write(&obj, sizeof(obj), 0);
}

void Drawable::Draw(const vk::CommandBuffer& cmdBuf) const
{
	GetMaterial().Bind(cmdBuf);

	m_ObjectConstantsDescriptorSet.value().Bind(Enums::value(BuiltinUniformBuffers::Set::Object), cmdBuf, GetMaterial().GetPipeline());

	GetMesh().Draw(cmdBuf);
}

void Drawable::CreateObjectConstants()
{
	m_ObjectConstantsBuffer = std::make_shared<UniformBuffer>(m_Device, sizeof(ObjectConstants), vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	std::shared_ptr<DescriptorSetCreateInfo> createInfo = std::make_shared<DescriptorSetCreateInfo>();

	createInfo->m_Data.push_back(DescriptorSetCreateInfo::Binding(
		0, vk::ShaderStageFlagBits::eAll, m_ObjectConstantsBuffer, "m_ObjectConstantsBuffer"));

	createInfo->m_Layout = m_Device.GetBuiltinUniformBuffers().GetDescriptorSetLayout(BuiltinUniformBuffers::Set::Object);

	m_ObjectConstantsDescriptorSet.emplace(m_Device, createInfo);
}
