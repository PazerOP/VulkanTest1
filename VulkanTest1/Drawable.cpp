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
	static const auto startTimePoint = std::chrono::high_resolution_clock::now();

	const float time = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - startTimePoint).count();

	ObjectConstants obj;

	Transform copy = m_Transform;

	const float jokeScale = Remap(600, 1000, -1, 1, sin(fmodf(time, 2 * 3.14159)));
	copy.SetScale(glm::vec2(jokeScale));

	obj.modelToWorld = copy.ComputeMatrix();
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
