#include "stdafx.h"
#include "DescriptorSet.h"

#include "Buffer.h"
#include "DescriptorSetCreateInfo.h"
#include "DescriptorSetLayout.h"
#include "DescriptorSetLayoutCreateInfo.h"
#include "LogicalDevice.h"
#include "Texture.h"

#include <forward_list>

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
	std::vector<vk::WriteDescriptorSet> descriptorWrites;

	// forward_list so the pointers don't move around (linked list)
	std::forward_list<vk::DescriptorBufferInfo> bufferInfos;
	std::forward_list<vk::DescriptorImageInfo> imageInfos;

	for (const auto& data : m_CreateInfo->m_Data)
	{
		const auto& foundBinding = std::find_if(bindings.begin(), bindings.end(),
												[&data](const auto& other) { return data.m_BindingIndex == other.binding; });
		assert(foundBinding != bindings.end());

		const auto& binding = *foundBinding;

		descriptorWrites.emplace_back();
		vk::WriteDescriptorSet& write = descriptorWrites.back();
		write.setDstSet(m_DescriptorSet.get());
		write.setDescriptorType(binding.descriptorType);
		write.setDescriptorCount(binding.descriptorCount);
		write.setDstBinding(binding.binding);

		switch (data.m_Data.index())
		{
		case 0:	// buffer
		{
			auto buffer = std::get<std::shared_ptr<Buffer>>(data.m_Data);

			bufferInfos.emplace_front();
			vk::DescriptorBufferInfo& bufferInfo = bufferInfos.front();

			bufferInfo.setBuffer(buffer->Get());
			bufferInfo.setRange(buffer->GetSize());
			bufferInfo.setOffset(buffer->GetOffset());

			write.setPBufferInfo(&bufferInfo);
			break;
		}
		case 1:	// texture
		{
			auto texture = std::get<std::shared_ptr<Texture>>(data.m_Data);

			imageInfos.emplace_front();
			vk::DescriptorImageInfo& imageInfo = imageInfos.front();

			//imageInfo.setImageLayout(texture->GetCreateInfo)
			imageInfo.setImageView(texture->GetImageView());
			imageInfo.setSampler(texture->GetSampler());

			write.setPImageInfo(&imageInfo);
			break;
		}
		}
	}

	std::array<vk::CopyDescriptorSet, 0> descriptorCopies;

	GetDevice()->updateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), descriptorCopies.size(), descriptorCopies.data());
}