#include "stdafx.h"
#include "MaterialDataManager.h"

#include "MaterialData.h"

MaterialDataManager::MaterialDataManager(LogicalDevice& device) :
	DataStoreType(device)
{
}

void MaterialDataManager::Reload()
{
	static const std::filesystem::path s_ShadersFolderPath(std::filesystem::current_path().append("materials"s));

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

		std::shared_ptr<const MaterialData> data(std::make_shared<MaterialData>(path));

		AddPair(data->GetName(), data);
	}
}