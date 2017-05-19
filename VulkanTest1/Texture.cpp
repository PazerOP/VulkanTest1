#include "stdafx.h"
#include "Texture.h"

#include "LogicalDevice.h"
#include "TextureCreateInfo.h"
#include "Vulkan.h"
#include "VulkanDebug.h"
#include "VulkanHelpers.h"

#include "stb_image.h"

#include <fstream>

Texture::~Texture()
{
	m_ImageView.reset();
	m_Image.reset();
	m_DeviceMemory.reset();
}

Texture::Texture(LogicalDevice& device, const std::shared_ptr<const TextureCreateInfo>& createInfo) :
	m_Device(device),
	m_CreateInfo(createInfo)
{
	Log::Msg<LogType::ObjectLifetime>(__FUNCSIG__);

	const auto& sourceImages = LoadSourceImages();
	const auto& firstImg = sourceImages.front();

	Buffer stagingBuffer(m_Device, firstImg.GetImgDataSize() * sourceImages.size(), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	{
		const size_t tempBufferSize = firstImg.GetImgDataSize();
		const size_t tempBufferStride = firstImg.GetImgDataStride();

		stagingBuffer.Write(firstImg.m_Image.get(), tempBufferSize, 0);

		if (sourceImages.size() > 1)
		{
			std::unique_ptr<uint8_t> tempBuffer((uint8_t*)malloc(tempBufferSize));
			for (size_t i = 1; i < sourceImages.size(); i++)
			{
				const auto& img = sourceImages[i];

				if (img.m_Width != firstImg.m_Width)
				{
					if (i > 1)
						memset(tempBuffer.get(), 0, tempBufferSize);

					const auto imgStride = img.GetImgDataStride();
					const auto minStride = std::min(imgStride, tempBufferStride);
					const auto minHeight = std::min(img.m_Height, firstImg.m_Height);

					// Copy line by line
					for (size_t y = 0; y < minHeight; y++)
						memcpy(tempBuffer.get() + tempBufferStride * y, (uint8_t*)img.m_Image.get() + imgStride * y, minStride);

					stagingBuffer.Write(tempBuffer.get(), tempBufferSize, tempBufferSize * i);
				}
				else
				{
					stagingBuffer.Write(img.m_Image.get(), tempBufferSize, tempBufferSize * i);
				}
			}
		}
	}

	// Setup final image
	{
		m_ImageCreateInfo.setImageType(sourceImages.size() == 1 ? vk::ImageType::e2D : vk::ImageType::e3D);
		m_ImageCreateInfo.setExtent(vk::Extent3D(firstImg.m_Width, firstImg.m_Height, sourceImages.size()));
		m_ImageCreateInfo.setMipLevels(1);
		m_ImageCreateInfo.setArrayLayers(1);
		m_ImageCreateInfo.setFormat(vk::Format::eR8G8B8A8Unorm);
		m_ImageCreateInfo.setTiling(vk::ImageTiling::eOptimal);
		m_ImageCreateInfo.setInitialLayout(vk::ImageLayout::ePreinitialized);
		m_ImageCreateInfo.setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
		m_ImageCreateInfo.setSharingMode(vk::SharingMode::eExclusive);

		m_Image = m_Device->createImageUnique(m_ImageCreateInfo);
	}

	// Alloc final device memory
	{
		const vk::MemoryRequirements memReqs = m_Device->getImageMemoryRequirements(m_Image.get());

		vk::MemoryAllocateInfo allocInfo;
		allocInfo.setAllocationSize(memReqs.size);
		allocInfo.setMemoryTypeIndex(m_Device.GetData().FindMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal));

		m_DeviceMemory = m_Device->allocateMemoryUnique(allocInfo);
	}

	m_Device->bindImageMemory(m_Image.get(), m_DeviceMemory.get(), 0);

	// Copy from staging to final
	{
		TransitionImageLayout(m_Image.get(), m_ImageCreateInfo.format, vk::ImageLayout::ePreinitialized, vk::ImageLayout::eTransferDstOptimal);
		CopyBufferToImage(stagingBuffer.Get(), m_Image.get(), m_ImageCreateInfo.extent);
		TransitionImageLayout(m_Image.get(), m_ImageCreateInfo.format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	}

	CreateImageView();
	CreateSampler();
}

std::vector<Texture::SourceImage> Texture::LoadSourceImages()
{
	std::vector<SourceImage> retVal;
	stbi_set_flip_vertically_on_load(true);

	for (const auto& path : m_CreateInfo->m_SourceFiles)
	{
		std::ifstream file(path.string(), std::ios::binary | std::ios::ate);
		assert(file.good());

		const auto length = file.tellg();
		std::shared_ptr<void> buffer(malloc(length));
		file.seekg(0);

		file.read(reinterpret_cast<char*>(buffer.get()), length);

		SourceImage newImg;
		newImg.m_Image.reset(stbi_load_from_memory(reinterpret_cast<stbi_uc*>(buffer.get()), length, &newImg.m_Width, &newImg.m_Height, &newImg.m_Channels, 4));

		if (newImg.m_Width <= 0 || newImg.m_Height <= 0 || newImg.m_Channels <= 0)
			throw std::runtime_error(StringTools::CSFormat("Failed to load raw img in {0}(): width {1}, height {2}, channels {3}, image pointer {4}",
														   __FUNCTION__, newImg.m_Width, newImg.m_Height, newImg.m_Channels, newImg.m_Image.get()));

		retVal.push_back(newImg);
	}

	return retVal;
}

void Texture::CreateImageView()
{
	if (m_ImageCreateInfo.extent.height > 1 && m_ImageCreateInfo.extent.depth > 1)
		m_ImageViewCreateInfo.setViewType(vk::ImageViewType::e3D);
	else if (m_ImageCreateInfo.extent.height > 1)
		m_ImageViewCreateInfo.setViewType(vk::ImageViewType::e2D);
	else
		m_ImageViewCreateInfo.setViewType(vk::ImageViewType::e1D);

	m_ImageViewCreateInfo.setImage(m_Image.get());
	m_ImageViewCreateInfo.setFormat(m_ImageCreateInfo.format);
	m_ImageViewCreateInfo.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
	m_ImageViewCreateInfo.subresourceRange.setLevelCount(1);
	m_ImageViewCreateInfo.subresourceRange.setLayerCount(1);

	m_ImageView = m_Device->createImageViewUnique(m_ImageViewCreateInfo);
}

void Texture::CreateSampler()
{
	m_SamplerCreateInfo.setMagFilter(m_CreateInfo->m_Filter);
	m_SamplerCreateInfo.setMinFilter(m_CreateInfo->m_Filter);

	m_SamplerCreateInfo.setAddressModeU(m_CreateInfo->m_AddressModeU);
	m_SamplerCreateInfo.setAddressModeV(m_CreateInfo->m_AddressModeV);
	m_SamplerCreateInfo.setAddressModeW(m_CreateInfo->m_AddressModeW);

	m_SamplerCreateInfo.setAnisotropyEnable(true);
	m_SamplerCreateInfo.setMaxAnisotropy(16);

	m_SamplerCreateInfo.setBorderColor(vk::BorderColor::eIntOpaqueBlack);

	m_SamplerCreateInfo.setUnnormalizedCoordinates(false);

	m_SamplerCreateInfo.setCompareEnable(false);
	m_SamplerCreateInfo.setCompareOp(vk::CompareOp::eAlways);

	m_SamplerCreateInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
	m_SamplerCreateInfo.setMipLodBias(0);
	m_SamplerCreateInfo.setMinLod(0);
	m_SamplerCreateInfo.setMaxLod(0);

	m_Sampler = m_Device->createSamplerUnique(m_SamplerCreateInfo);
}

void Texture::TransitionImageLayout(const vk::Image& img, vk::Format /*format*/, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const
{
	auto cmdBuf = m_Device.AllocCommandBuffer();
	cmdBuf->begin(VulkanHelpers::CBBI_ONE_TIME_SUBMIT);
	{
		vk::ImageMemoryBarrier barrier;
		barrier.setOldLayout(oldLayout);
		barrier.setNewLayout(newLayout);

		barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
		barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);

		barrier.setImage(img);
		barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
		barrier.subresourceRange.setLevelCount(1);
		barrier.subresourceRange.setLayerCount(1);

		if (oldLayout == vk::ImageLayout::ePreinitialized && newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.setSrcAccessMask(vk::AccessFlagBits::eHostWrite);
			barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
			barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
		}
		else
			throw std::invalid_argument("Unsupported layout transition");

		cmdBuf->pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTopOfPipe,
			(vk::DependencyFlagBits)0,
			nullptr, nullptr,
			barrier);
	}
	cmdBuf->end();
	m_Device.SubmitCommandBuffers(cmdBuf.get());
}

void Texture::CopyBufferToImage(const vk::Buffer& src, const vk::Image& dst, const vk::Extent3D& extent) const
{
	vk::UniqueCommandBuffer cmdBuf = m_Device.AllocCommandBuffer();
	cmdBuf->begin(VulkanHelpers::CBBI_ONE_TIME_SUBMIT);

	vk::BufferImageCopy region;

	region.imageSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
	region.imageSubresource.setLayerCount(1);

	//region.setImageOffset(vk::Offset3D(0, 0, 0));
	region.setImageExtent(extent);

	cmdBuf->copyBufferToImage(src, dst, vk::ImageLayout::eTransferDstOptimal, region);

	cmdBuf->end();
	m_Device.SubmitCommandBuffers(cmdBuf.get());
}
