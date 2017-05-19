#pragma once
#include "Util.h"

#include <filesystem>
#include <string>
#include <variant>
#include <vector>

struct TextureCreateInfo
{
	TextureCreateInfo();

	std::filesystem::path m_DefinitionFile;

	std::vector<std::filesystem::path> m_SourceFiles;

	vk::Filter m_Filter;
	bool m_Animated;

	vk::SamplerAddressMode m_AddressModeU;
	vk::SamplerAddressMode m_AddressModeV;
	vk::SamplerAddressMode m_AddressModeW;
};