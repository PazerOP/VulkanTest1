#include "stdafx.h"
#include "Texture.h"

#include "LogicalDevice.h"
#include "TextureCreateInfo.h"
#include "Vulkan.h"
#include "VulkanDebug.h"
#include "VulkanHelpers.h"

#include "stb_image.h"

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

	int w, h, channels;
	auto rawImg = std::shared_ptr<stbi_uc>(stbi_load(GetCreateInfo().m_Path.string().c_str(), &w, &h, &channels, STBI_rgb_alpha),
										   [](stbi_uc* i) { stbi_image_free(i); });
	if (w <= 0 || h <= 0 || channels <= 0)
		throw std::runtime_error(StringTools::CSFormat("Failed to load raw img in texture constructor: width {0}, height {1}, channels {2}", w, h, channels));

	const size_t rawImgSize = w * h * channels;

	// Create staging image
	vk::UniqueImage stagingImage;
	{
		m_ImageCreateInfo.setImageType(vk::ImageType::e2D);
		m_ImageCreateInfo.setExtent(vk::Extent3D(w, h, 1));
		m_ImageCreateInfo.setMipLevels(1);
		m_ImageCreateInfo.setArrayLayers(1);
		m_ImageCreateInfo.setFormat(vk::Format::eR8G8B8A8Unorm);
		m_ImageCreateInfo.setTiling(vk::ImageTiling::eLinear);
		m_ImageCreateInfo.setInitialLayout(vk::ImageLayout::ePreinitialized);
		m_ImageCreateInfo.setUsage(vk::ImageUsageFlagBits::eTransferSrc);
		m_ImageCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
		m_ImageCreateInfo.setSamples(vk::SampleCountFlagBits::e1);

		stagingImage = m_Device->createImageUnique(m_ImageCreateInfo);
	}

	// Allocate staging memory
	vk::UniqueDeviceMemory stagingMemory;
	vk::DeviceSize stagingMemorySize;
	{
		const vk::MemoryRequirements memReqs = m_Device->getImageMemoryRequirements(stagingImage.get());

		stagingMemorySize = memReqs.size;

		vk::MemoryAllocateInfo allocInfo;
		allocInfo.setAllocationSize(stagingMemorySize);
		allocInfo.setMemoryTypeIndex(m_Device.GetData().FindMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));

		stagingMemory = m_Device->allocateMemoryUnique(allocInfo);
	}

	m_Device->bindImageMemory(stagingImage.get(), stagingMemory.get(), 0);

	// Copy raw img to staging memory
	{
		vk::ImageSubresource subresource;
		subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);

		const vk::SubresourceLayout stagingImageLayout = m_Device->getImageSubresourceLayout(stagingImage.get(), subresource);

		void* data = m_Device->mapMemory(stagingMemory.get(), 0, stagingMemorySize);
		if (stagingImageLayout.rowPitch == vk::DeviceSize(w * channels))
			memcpy_s(data, stagingMemorySize, rawImg.get(), rawImgSize);
		else
		{
			uint8_t* startPos = reinterpret_cast<uint8_t*>(data);
			for (int y = 0; y < h; y++)
			{
				uint8_t* const writePos = &startPos[y * stagingImageLayout.rowPitch];
				memcpy_s(writePos,
						 stagingMemorySize - (writePos - startPos),
						 &rawImg.get()[y * w * channels],
						 w * channels);
			}
		}
		m_Device->unmapMemory(stagingMemory.get());
	}

	// Free raw img
	rawImg.reset();

	// Setup final image
	{
		m_ImageCreateInfo.setTiling(vk::ImageTiling::eOptimal);
		m_ImageCreateInfo.setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);

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
		TransitionImageLayout(stagingImage.get(), m_ImageCreateInfo.format, m_ImageCreateInfo.initialLayout, vk::ImageLayout::eTransferSrcOptimal);
		TransitionImageLayout(m_Image.get(), m_ImageCreateInfo.format, m_ImageCreateInfo.initialLayout, vk::ImageLayout::eTransferDstOptimal);
		CopyImage(stagingImage.get(), m_Image.get(), w, h);
	}

	stagingImage.reset();
	stagingMemory.reset();

	CreateImageView();
	CreateSampler();
}

void Texture::CreateImageView()
{
	m_ImageViewCreateInfo.setImage(m_Image.get());
	m_ImageViewCreateInfo.setViewType(vk::ImageViewType::e2D);
	m_ImageViewCreateInfo.setFormat(m_ImageCreateInfo.format);
	m_ImageViewCreateInfo.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
	m_ImageViewCreateInfo.subresourceRange.setLevelCount(1);
	m_ImageViewCreateInfo.subresourceRange.setLayerCount(1);

	m_ImageView = m_Device->createImageViewUnique(m_ImageViewCreateInfo);
}

void Texture::CreateSampler()
{
	m_SamplerCreateInfo.setMagFilter(vk::Filter::eLinear);
	m_SamplerCreateInfo.setMinFilter(vk::Filter::eLinear);

	m_SamplerCreateInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat);
	m_SamplerCreateInfo.setAddressModeV(vk::SamplerAddressMode::eRepeat);
	m_SamplerCreateInfo.setAddressModeW(vk::SamplerAddressMode::eRepeat);

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

void Texture::TransitionImageLayout(const vk::Image& img, vk::Format /*format*/, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
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

		if (oldLayout == vk::ImageLayout::ePreinitialized && newLayout == vk::ImageLayout::eTransferSrcOptimal)
		{
			barrier.setSrcAccessMask(vk::AccessFlagBits::eHostWrite);
			barrier.setDstAccessMask(vk::AccessFlagBits::eTransferRead);
		}
		else if (oldLayout == vk::ImageLayout::ePreinitialized && newLayout == vk::ImageLayout::eTransferDstOptimal)
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

void Texture::CopyImage(const vk::Image& src, const vk::Image& dst, uint32_t width, uint32_t height)
{
	vk::UniqueCommandBuffer cmdBuf = m_Device.AllocCommandBuffer();
	cmdBuf->begin(VulkanHelpers::CBBI_ONE_TIME_SUBMIT);

	vk::ImageSubresourceLayers subresourceLayers;
	subresourceLayers.setAspectMask(vk::ImageAspectFlagBits::eColor);
	subresourceLayers.setLayerCount(1);

	vk::ImageCopy region;
	region.setSrcSubresource(subresourceLayers);
	region.setDstSubresource(subresourceLayers);
	region.extent.width = width;
	region.extent.height = height;
	region.extent.depth = 1;

	cmdBuf->copyImage(src, vk::ImageLayout::eTransferSrcOptimal,
					  dst, vk::ImageLayout::eTransferDstOptimal,
					  region);

	cmdBuf->end();
	m_Device->waitIdle();
}
