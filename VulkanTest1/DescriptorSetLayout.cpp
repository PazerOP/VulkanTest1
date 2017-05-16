#include "stdafx.h"
#include "DescriptorSetLayout.h"

#include "DescriptorSetLayoutCreateInfo.h"
#include "LogicalDevice.h"

DescriptorSetLayout::DescriptorSetLayout(LogicalDevice& device, const std::shared_ptr<const DescriptorSetLayoutCreateInfo>& createInfo) :
	m_Device(device),
	m_CreateInfo(createInfo)
{
	CreateDescriptorSetLayout();
}

void DescriptorSetLayout::CreateDescriptorSetLayout()
{
	std::vector<vk::DescriptorSetLayoutBinding> bindings;
	for (const auto& binding : m_CreateInfo->m_Bindings)
		bindings.push_back(binding);

	vk::DescriptorSetLayoutCreateInfo createInfo;
	createInfo.setBindingCount(bindings.size());
	createInfo.setPBindings(bindings.data());

	m_Layout = GetDevice()->createDescriptorSetLayoutUnique(createInfo);
}
