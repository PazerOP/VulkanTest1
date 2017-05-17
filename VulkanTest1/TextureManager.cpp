#include "stdafx.h"
#include "TextureManager.h"

#include "JSON.h"
#include "Texture.h"
#include "TextureCreateInfo.h"

#include <filesystem>

static const std::filesystem::path s_ShadersFolderPath(std::filesystem::current_path().append("textures"s));

TextureManager::TextureManager(LogicalDevice& device) :
	DataStoreType(device)
{
}

void TextureManager::Reload()
{
	ClearData();

	for (auto& item : std::filesystem::recursive_directory_iterator(s_ShadersFolderPath))
	{
		const auto& type = item.status().type();
		if (type != std::filesystem::file_type::regular)
			continue;

		const auto& path = item.path();
		if (!path.has_extension())
			continue;

		const auto& extension = path.extension();
		if (extension != ".json")
			continue;
		
		auto name = path.string().erase(0, s_ShadersFolderPath.string().size() + 1);

		// Remove file extension
		name.erase(name.size() - extension.string().size());

		for (char& c : name)
		{
			if (c == '\\')
				c = '/';
		}

		AddPair(name, LoadCreateInfo(path));
	}
}

std::shared_ptr<Texture> TextureManager::Transform(const std::shared_ptr<TextureCreateInfo>& createInfo) const
{
	return std::make_shared<Texture>(m_Device, createInfo);
}

std::shared_ptr<TextureCreateInfo> TextureManager::LoadCreateInfo(const std::filesystem::path& path)
{
	std::shared_ptr<TextureCreateInfo> retVal = std::make_shared<TextureCreateInfo>();

	JSONObject obj = JSONSerializer::FromFile(path).GetObject();

	const std::string& relativeFilename = obj.find("fileName")->second.GetString();

	retVal->m_Path = s_ShadersFolderPath / relativeFilename;

	return retVal;
}
