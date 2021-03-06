#include "stdafx.h"
#include "ShaderGroupDataManager.h"

#include "ContentPaths.h"
#include "ShaderGroupData.h"

#include <filesystem>

ShaderGroupDataManager::ShaderGroupDataManager(LogicalDevice& device) :
	DataStoreType(device)
{
}

void ShaderGroupDataManager::Reload()
{
	ClearData();

	for (auto& item : std::filesystem::recursive_directory_iterator(ContentPaths::Shaders()))
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

		std::shared_ptr<const ShaderGroupData> data(std::make_shared<ShaderGroupData>(path));

		AddPair(name_from_path(ContentPaths::Shaders(), data->GetName()), data);
	}
}