#include "stdafx.h"
#include "DescriptorSet.h"

#include "Buffer.h"
#include "DescriptorSetCreateInfo.h"
#include "DescriptorSetLayout.h"
#include "DescriptorSetLayoutCreateInfo.h"
#include "LogicalDevice.h"

DescriptorSet::DescriptorSet(LogicalDevice& device, const std::shared_ptr<const DescriptorSetCreateInfo>& createInfo) :
	m_Device(device),
	m_CreateInfo(createInfo)
{
	Log::Msg<LogType::ObjectLifetime>(__FUNCSIG__);

	if (!m_CreateInfo)
		throw std::invalid_argument("createInfo == nullptr");

	CreateDescriptorSet();
}

void DescriptorSet::Bind(uint32_t setID, const vk::CommandBuffer& cmdBuf, const GraphicsPipeline& pipeline) const
{
	auto sets = make_array<vk::DescriptorSet>(m_DescriptorSet.get());
	std::array<uint32_t, 0> dynamicOffsets;
	cmdBuf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.GetPipelineLayout(), setID, sets, dynamicOffsets);
}

void DescriptorSet::CreateDescriptorSet()
{
	auto layouts = make_array<vk::DescriptorSetLayout>(m_CreateInfo->m_Layout->Get());

	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.setDescriptorPool(GetDevice().GetDescriptorPool());
	allocInfo.setDescriptorSetCount(layouts.size());
	allocInfo.setPSetLayouts(layouts.data());

	m_DescriptorSet = std::move(GetDevice()->allocateDescriptorSetsUnique(allocInfo).front());

	const auto& bindings = m_CreateInfo->m_Layout->GetCreateInfo()->m_Bindings;
	std::vector<vk::WriteDescriptorSet> descriptorWrites(bindings.size());
	std::vector<vk::DescriptorBufferInfo> bufferInfos(bindings.size());
	
	for (size_t i = 0; i < bindings.size(); i++)
	{
		const auto& binding = bindings[i];

		vk::DescriptorBufferInfo& bufferInfo = bufferInfos[i];

		auto buffer = std::get<std::shared_ptr<Buffer>>(m_CreateInfo->m_Data[i]);
		bufferInfo.setBuffer(buffer->GetBuffer());
		bufferInfo.setRange(buffer->GetCreateInfo().size);

		vk::WriteDescriptorSet& write = descriptorWrites[i];
		write.setDstSet(m_DescriptorSet.get());
		write.setDescriptorType(binding.descriptorType);
		write.setDescriptorCount(binding.descriptorCount);
		write.setDstBinding(i);
		write.setPBufferInfo(&bufferInfo);
	}

	std::array<vk::CopyDescriptorSet, 0> descriptorCopies;

	GetDevice()->updateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), descriptorCopies.size(), descriptorCopies.data());
}