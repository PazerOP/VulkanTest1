#pragma once
#include <filesystem>
#include <optional>

class LogicalDevice;

class Texture
{
public:
	static std::unique_ptr<Texture> Create(const std::filesystem::path& imgPath, LogicalDevice* device = nullptr);

private:
	Texture(const std::filesystem::path& imgPath, LogicalDevice* device = nullptr);

	void TransitionImageLayout(const vk::Image& img, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
	void CopyImage(const vk::Image& src, const vk::Image& dst, uint32_t width, uint32_t height);

	LogicalDevice& m_Device;

	vk::UniqueImage m_Image;
	vk::UniqueDeviceMemory m_DeviceMemory;

	std::filesystem::path m_ImagePath;
};