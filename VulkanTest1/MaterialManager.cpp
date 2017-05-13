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
	m_Data.clear();

	for (const auto& entry : MaterialDataManager::Instance())
	{
		const auto& data = entry.second;

		m_Data.insert(std::make_pair(data->GetName(), std::make_shared<Material>(data, m_Device)));
	}
}
