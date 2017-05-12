#include "stdafx.h"
#include "ShaderGroupManager.h"

#include "ShaderGroup.h"
#include "ShaderGroupData.h"
#include "ShaderGroupDataManager.h"

ShaderGroupManager* ShaderGroupManager::s_Instance;

ShaderGroupManager::ShaderGroupManager(LogicalDevice& device) :
	m_Device(device)
{
	Reload();

	assert(!s_Instance);
	s_Instance = this;
}

ShaderGroupManager::~ShaderGroupManager()
{
	assert(s_Instance == this);
	s_Instance = nullptr;
}

ShaderGroupManager& ShaderGroupManager::Instance()
{
	if (!s_Instance)
		throw std::runtime_error("Attempted to call " __FUNCSIG__ " before the ShaderGroupManager instance was constructed!");

	return *s_Instance;
}

void ShaderGroupManager::Reload()
{
	m_ShaderGroups.clear();

	for (const auto& entry : ShaderGroupDataManager::Instance())
	{
		const auto& data = entry.second;
		
		m_ShaderGroups.insert(std::make_pair(data->GetName(), std::make_shared<ShaderGroup>(data, m_Device)));
	}
}

std::shared_ptr<const ShaderGroup> ShaderGroupManager::FindShaderGroup(const std::string& name) const
{
	auto found = m_ShaderGroups.find(name);
	if (found != m_ShaderGroups.end())
		return found->second;

	Log::Msg(__FUNCTION__ ": Unable to find a ShaderGroup named \"{0}\"", name);
	return nullptr;
}
