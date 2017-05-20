#include "stdafx.h"
#include "TextureManager.h"

#include "ContentPaths.h"
#include "JSON.h"
#include "Texture.h"
#include "TextureCreateInfo.h"

#include <filesystem>

TextureManager::TextureManager(LogicalDevice& device) :
	DataStoreType(device)
{
}

void TextureManager::Reload()
{
	ClearData();

	for (auto& item : std::filesystem::recursive_directory_iterator(ContentPaths::Textures()))
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

		const auto name = name_from_path(ContentPaths::Textures(), item);

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

	const JSONObject root = JSONSerializer::FromFile(path).GetObject();

	for (const auto& sourceFile : root.GetArray("sourceFiles"))
		retVal->m_SourceFiles.push_back(ContentPaths::Textures() / sourceFile.GetString());

	retVal->m_Animated = root.TryGetBool("animated", false);

	retVal->m_Filter = ToFilter(root.TryGetString("filter", "linear"));

	LoadAddressMode(*retVal, root);

	return retVal;
}

void TextureManager::LoadAddressMode(TextureCreateInfo& createInfo, const JSONObject& root)
{
	const JSONValue* addressMode = root.TryGetValue("addressMode");
	if (!addressMode)
		return;

	static const std::map<std::string, vk::SamplerAddressMode> s_AddressModeLookup =
	{
		{ "repeat", vk::SamplerAddressMode::eRepeat },
		{ "repeatMirror", vk::SamplerAddressMode::eMirroredRepeat },
		{ "clampEdge", vk::SamplerAddressMode::eClampToEdge },
		{ "clampEdgeMirror", vk::SamplerAddressMode::eMirrorClampToEdge },
		{ "clampBorder", vk::SamplerAddressMode::eClampToBorder },
	};

	if (addressMode->GetType() == JSONDataType::Object)
	{
		const JSONObject& addressModeObj = addressMode->GetObject();
		createInfo.m_AddressModeU = vk::SamplerAddressMode(s_AddressModeLookup.at(addressModeObj.TryGetString("u", s_AddressModeLookup.begin()->first)));
		createInfo.m_AddressModeV = vk::SamplerAddressMode(s_AddressModeLookup.at(addressModeObj.TryGetString("v", s_AddressModeLookup.begin()->first)));
		createInfo.m_AddressModeW = vk::SamplerAddressMode(s_AddressModeLookup.at(addressModeObj.TryGetString("w", s_AddressModeLookup.begin()->first)));
	}
	else if (addressMode->GetType() == JSONDataType::String)
	{
		createInfo.m_AddressModeU = createInfo.m_AddressModeV = createInfo.m_AddressModeW =
			vk::SamplerAddressMode(s_AddressModeLookup.at(addressMode->GetString()));
	}
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
