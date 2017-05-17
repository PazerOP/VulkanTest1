#include "stdafx.h"
#include "MaterialManager.h"

#include "Material.h"
#include "MaterialData.h"
#include "MaterialDataManager.h"

MaterialManager::MaterialManager(LogicalDevice& device) :
	DataStoreType(device)
{
}

void MaterialManager::Reload()
{
	ClearData();

	for (const auto& entry : MaterialDataManager::Instance())
	{
		const auto& data = entry.second.Get();

		AddPair(data->GetName(), std::make_shared<Material>(data, m_Device));
	}
}

void MaterialManager::RecreatePipelines()
{
	for (auto& entry : Instance())
	{
		entry.second.Get()->GetPipeline().RecreatePipeline();
	}
}
