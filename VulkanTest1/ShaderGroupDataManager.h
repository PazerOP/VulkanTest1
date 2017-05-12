#pragma once

#include <map>
#include <memory>

class ShaderGroupData;

class ShaderGroupDataManager
{
public:
	ShaderGroupDataManager();
	~ShaderGroupDataManager();
	static ShaderGroupDataManager& Instance();

	void ReloadData();

	std::shared_ptr<const ShaderGroupData> FindData(const std::string& name) const;

	auto begin() const { return m_Data.begin(); }
	auto begin() { return m_Data.begin(); }
	auto end() const { return m_Data.end(); }
	auto end() { return m_Data.end(); }

private:
	static ShaderGroupDataManager* s_Instance;

	std::map<std::string, std::shared_ptr<const ShaderGroupData>> m_Data;
};