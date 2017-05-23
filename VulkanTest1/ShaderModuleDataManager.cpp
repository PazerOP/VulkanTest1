#include "stdafx.h"
#include "ShaderModuleDataManager.h"

#include "ContentPaths.h"
#include "ShaderModuleData.h"

#include <filesystem>

ShaderModuleDataManager::ShaderModuleDataManager(LogicalDevice& device) :
	DataStore(device)
{
}

void ShaderModuleDataManager::Reload()
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
		if (extension != ".spv")
			continue;

		std::shared_ptr<const ShaderModuleData> data(std::make_shared<ShaderModuleData>(path));

		AddPair(data->GetName(), data);
	}
}
