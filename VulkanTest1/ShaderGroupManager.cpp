#include "stdafx.h"
#include "ShaderGroupManager.h"

#include "ShaderGroup.h"
#include "ShaderGroupData.h"
#include "ShaderGroupDataManager.h"

ShaderGroupManager::ShaderGroupManager(LogicalDevice& device) :
	DataStoreType(device)
{
}

void ShaderGroupManager::Reload()
{
	m_Data.clear();

	for (const auto& entry : ShaderGroupDataManager::Instance())
	{
		const auto& data = entry.second;
		
		m_Data.insert(std::make_pair(data->GetName(), std::make_shared<ShaderGroup>(data, m_Device)));
	}
}