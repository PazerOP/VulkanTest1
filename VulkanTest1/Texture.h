#pragma once
#include <filesystem>
#include <optional>

class LogicalDevice;
struct TextureCreateInfo;

class Texture
{
public:
	Texture(LogicalDevice& device, const std::shared_ptr<const TextureCreateInfo>& createInfo);
	~Texture();

	const std::shared_ptr<const TextureCreateInfo>& GetCreateInfoPtr() const { return m_CreateInfo; }
	const TextureCreateInfo& GetCreateInfo() const { return *m_CreateInfo; }

	vk::ImageView GetImageView() const { return m_ImageView.get(); }
	vk::Sampler GetSampler() const { return m_Sampler.get(); }

	const vk::ImageType GetImageType() const { return m_ImageCreateInfo.imageType; }

private:
	struct SourceImage
	{
		std::shared_ptr<void> m_Image;
		size_t GetImgDataSize() const { return m_Width * m_Height * 4; }
		size_t GetImgDataStride() const { return m_Width * 4; }
		int m_Width;
		int m_Height;
		int m_Channels;
	};
	std::vector<SourceImage> LoadSourceImages();

	void CreateImageView();
	void CreateSampler();

	void TransitionImageLayout(const vk::Image& img, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const;
	void CopyBufferToImage(const vk::Buffer& src, const vk::Image& dst, const vk::Extent3D& extent) const;

	LogicalDevice& m_Device;

	vk::ImageCreateInfo m_ImageCreateInfo;
	vk::ImageViewCreateInfo m_ImageViewCreateInfo;
	vk::UniqueImage m_Image;
	vk::UniqueDeviceMemory m_DeviceMemory;
	vk::UniqueImageView m_ImageView;

	vk::SamplerCreateInfo m_SamplerCreateInfo;
	vk::UniqueSampler m_Sampler;

	std::shared_ptr<const TextureCreateInfo> m_CreateInfo;
};