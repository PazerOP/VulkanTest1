#include "stdafx.h"
#include "ShaderGroupData.h"
#include "ShaderGroupDataManager.h"

#include <filesystem>

ShaderGroupDataManager::ShaderGroupDataManager(LogicalDevice& device) :
	DataStoreType(device)
{
}

void ShaderGroupDataManager::Reload()
{
	ClearData();

	static const std::filesystem::path s_ShadersFolderPath(std::filesystem::current_path().append("shaders"s));

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

		std::shared_ptr<const ShaderGroupData> data(std::make_shared<ShaderGroupData>(path));

		AddPair(data->GetName(), data);
	}
}