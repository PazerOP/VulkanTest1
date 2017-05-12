#include "stdafx.h"
#include "ShaderGroupData.h"
#include "ShaderGroupDataManager.h"

#include <filesystem>

ShaderGroupDataManager* ShaderGroupDataManager::s_Instance;

static const std::filesystem::path s_ShadersFolderPath(std::filesystem::current_path().append("shaders"s));

ShaderGroupDataManager::ShaderGroupDataManager()
{
	ReloadData();

	assert(!s_Instance);
	s_Instance = this;
}

ShaderGroupDataManager::~ShaderGroupDataManager()
{
	assert(s_Instance == this);
	s_Instance = nullptr;
}

ShaderGroupDataManager& ShaderGroupDataManager::Instance()
{
	if (!s_Instance)
		throw std::runtime_error("Attempted to call " __FUNCSIG__ " before the ShaderGroupDataManager instance was constructed!");

	return *s_Instance;
}

void ShaderGroupDataManager::ReloadData()
{
	m_Data.clear();

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

		m_Data.insert(std::make_pair(data->GetName(), data));
	}
}

std::shared_ptr<const ShaderGroupData> ShaderGroupDataManager::FindData(const std::string& name) const
{
	auto found = m_Data.find(name);
	if (found != m_Data.end())
		return found->second;

	Log::Msg(__FUNCTION__ ": Unable to find a ShaderGroupData named \"{0}\"", name);
	return nullptr;
}
