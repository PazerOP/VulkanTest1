#pragma once
#include <filesystem>

class ContentPaths
{
public:
	static const std::filesystem::path& Textures();
	static const std::filesystem::path& Materials();
	static const std::filesystem::path& Shaders();
};