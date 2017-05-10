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
	auto rawImg = std::unique_ptr<stbi_uc>(stbi_load(imgPath.string().c_str(), &w, &h, &channels, STBI_rgb_alpha));
	const size_t rawImgSize = w * h * channels;

	// Create staging image
	vk::UniqueImage stagingImage;
	vk::ImageCreateInfo imgCreateInfo;
	{
		imgCreateInfo.setImageType(vk::ImageType::e2D);
		imgCreateInfo.setExtent(vk::Extent3D(w, h, 1));
		imgCreateInfo.setMipLevels(1);
		imgCreateInfo.setArrayLayers(1);
		imgCreateInfo.setFormat(vk::Format::eR8G8B8A8Unorm);
		imgCreateInfo.setTiling(vk::ImageTiling::eLinear);
		imgCreateInfo.setInitialLayout(vk::ImageLayout::ePreinitialized);
		imgCreateInfo.setUsage(vk::ImageUsageFlagBits::eTransferSrc);
		imgCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
		imgCreateInfo.setSamples(vk::SampleCountFlagBits::e1);

		stagingImage = m_Device->createImageUnique(imgCreateInfo);
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
		if (stagingImageLayout.rowPitch == w * channels)
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
		imgCreateInfo.setTiling(vk::ImageTiling::eOptimal);
		imgCreateInfo.setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);

		m_Image = m_Device->createImageUnique(imgCreateInfo);
	}

	// Alloc final device memory
	{
		const vk::MemoryRequirements memReqs = m_Device->getImageMemoryRequirements(m_Image.get());

		vk::MemoryAllocateInfo allocInfo;
		allocInfo.setAllocationSize(memReqs.size);
		allocInfo.setMemoryTypeIndex(m_Device.GetData().FindMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal));

		stagingMemory = m_Device->allocateMemoryUnique(allocInfo);
	}

	// Copy from staging to final
	{
		vk::UniqueCommandBuffer copyCmdBuf = m_Device.AllocCommandBuffer();
		copyCmdBuf->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
		{
			vk::ImageMemoryBarrier barrier;
			//barrier.setOldLayout(vk::ImageLayout::)
		}
		copyCmdBuf->end();
		m_Device.SubmitCommandBuffers(copyCmdBuf.get());
	}
}