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

private:
	void CreateImageView();
	void CreateSampler();

	void TransitionImageLayout(const vk::Image& img, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
	void CopyImage(const vk::Image& src, const vk::Image& dst, uint32_t width, uint32_t height);

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