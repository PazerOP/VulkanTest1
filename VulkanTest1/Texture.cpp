#include "stdafx.h"
#include "Texture.h"

#include "LogicalDevice.h"
#include "stb_image.h"
#include "Vulkan.h"

std::unique_ptr<Texture> Texture::Create(const std::filesystem::path& imgPath, LogicalDevice* device)
{
	return std::unique_ptr<Texture>(new Texture(imgPath, device));
}

Texture::Texture(const std::filesystem::path& imgPath, LogicalDevice* device) :
	m_Device(device ? *device : Vulkan().GetLogicalDevice())
{
	m_ImagePath = imgPath;

	int w, h, channels;
	const auto rawImg = std::unique_ptr<stbi_uc>(stbi_load(imgPath.string().c_str(), &w, &h, &channels, STBI_rgb_alpha));
	const size_t rawImgSize = w * h * channels;

	vk::ImageCreateInfo createInfo;
	createInfo.setImageType(vk::ImageType::e2D);
	createInfo.setExtent(vk::Extent3D(w, h, 1));
	createInfo.setMipLevels(1);
	createInfo.setArrayLayers(1);
	createInfo.setFormat(vk::Format::eR8G8B8A8Unorm);
	createInfo.setTiling(vk::ImageTiling::eLinear);
	createInfo.setInitialLayout(vk::ImageLayout::ePreinitialized);
	createInfo.setUsage(vk::ImageUsageFlagBits::eTransferSrc);
	createInfo.setSharingMode(vk::SharingMode::eExclusive);
	createInfo.setSamples(vk::SampleCountFlagBits::e1);

	vk::UniqueImage stagingImage = m_Device->createImageUnique(createInfo);

	const vk::MemoryRequirements memReqs = m_Device->getImageMemoryRequirements(stagingImage.get());

	vk::MemoryAllocateInfo allocInfo;
	allocInfo.setAllocationSize(memReqs.size);
	allocInfo.setMemoryTypeIndex(m_Device.GetData().FindMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));

	vk::UniqueDeviceMemory stagingMemory = m_Device->allocateMemoryUnique(allocInfo);

	m_Device->bindImageMemory(stagingImage.get(), stagingMemory.get(), 0);

	vk::ImageSubresource subresource;
	subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);

	const vk::SubresourceLayout stagingImageLayout = m_Device->getImageSubresourceLayout(stagingImage.get(), subresource);

	void* data = m_Device->mapMemory(stagingMemory.get(), 0, allocInfo.allocationSize);
	if (stagingImageLayout.rowPitch == w * channels)
		memcpy_s(data, allocInfo.allocationSize, rawImg.get(), rawImgSize);
	else
	{
		uint8_t* startPos = reinterpret_cast<uint8_t*>(data);
		for (int y = 0; y < h; y++)
		{
			uint8_t* const writePos = &startPos[y * stagingImageLayout.rowPitch];
			memcpy_s(writePos,
					 allocInfo.allocationSize - (writePos - startPos),
					 &rawImg.get()[y * w * channels],
					 w * channels);
		}
	}
	m_Device->unmapMemory(stagingMemory.get());
}
