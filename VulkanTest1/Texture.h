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

	LogicalDevice& m_Device;

	vk::UniqueImage m_Image;
	vk::UniqueDeviceMemory m_DeviceMemory;

	std::filesystem::path m_ImagePath;
};