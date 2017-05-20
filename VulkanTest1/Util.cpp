#include "stdafx.h"

#include <filesystem>

std::string name_from_path(const std::filesystem::path& basePath, const std::filesystem::path& fullPath, bool removeExt)
{
	auto name = fullPath.string().erase(0, basePath.string().size() + 1);

	if (removeExt && fullPath.has_extension())
	{
		// Remove file extension
		name.erase(name.size() - fullPath.extension().string().size());
	}

	for (char& c : name)
	{
		if (c == '\\')
			c = '/';
	}

	return name;
}