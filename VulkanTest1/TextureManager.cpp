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

		Log::TagMsg(TAG, "Loading texture {0}", name);

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
	retVal->m_DefinitionFile = path;

	JSONObject obj = JSONSerializer::FromFile(path).GetObject();

	const auto& sourceFiles = obj.find("sourceFiles");
	if (sourceFiles == obj.end())
		throw json_parsing_error(StringTools::CSFormat("Failed to find required key \"sourceFiles\" in \"{0}\"", path));
	else
	{
		for (const auto& sourceFile : sourceFiles->second.GetArray())
			retVal->m_SourceFiles.push_back(s_ShadersFolderPath / sourceFile.GetString());
	}

	retVal->m_Animated = obj.TryGetBool("animated", false);

	retVal->m_Filter = ToFilter(obj.TryGetString("filter", "linear"));

	return retVal;
}

vk::Filter TextureManager::ToFilter(const std::string& filterText)
{
	if (filterText == "linear")
		return vk::Filter::eLinear;
	else if (filterText == "nearest")
		return vk::Filter::eNearest;
	else
		throw json_parsing_error(StringTools::CSFormat("Failed to convert \"{0}\" to a vk::Filter value", filterText));
}
