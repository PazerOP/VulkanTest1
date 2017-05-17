#pragma once
#include "DataStore.h"
#include <filesystem>

class LogicalDevice;
class Texture;
struct TextureCreateInfo;

class TextureManager : public DataStore<TextureManager, Texture, TextureCreateInfo>
{
public:
	TextureManager(LogicalDevice& device);

	void Reload() override;

private:
	std::shared_ptr<Texture> Transform(const std::shared_ptr<TextureCreateInfo>& createInfo) const override;

	static std::shared_ptr<TextureCreateInfo> LoadCreateInfo(const std::filesystem::path& path);
};