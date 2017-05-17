#pragma once
#include "Util.h"

#include <filesystem>

struct TextureCreateInfo
{
	std::filesystem::path m_Path;

	vk::Filter m_Filter;
};