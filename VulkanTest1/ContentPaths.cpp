#include "stdafx.h"
#include "ContentPaths.h"

const std::filesystem::path& ContentPaths::Textures()
{
	static const std::filesystem::path s_TexturesFolderPath(std::filesystem::current_path().append("textures"s));
	return s_TexturesFolderPath;
}

const std::filesystem::path& ContentPaths::Materials()
{
	static const std::filesystem::path s_MaterialsFolderPath(std::filesystem::current_path().append("materials"s));
	return s_MaterialsFolderPath;
}

const std::filesystem::path& ContentPaths::Shaders()
{
	static const std::filesystem::path s_ShadersFolderPath(std::filesystem::current_path().append("shaders"s));
	return s_ShadersFolderPath;
}
