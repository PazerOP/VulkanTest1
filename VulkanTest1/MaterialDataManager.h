#pragma once

#include <map>
#include <memory>

class MaterialData;

class MaterialDataManager
{
public:
	MaterialDataManager();
	~MaterialDataManager();
	static MaterialDataManager& Instance();

	void ReloadData();

	std::shared_ptr<const MaterialData> FindData(const std::string& name) const;

	auto begin() const { return m_Data.begin(); }
	auto begin() { return m_Data.begin(); }
	auto end() const { return m_Data.end(); }
	auto end() { return m_Data.end(); }

private:
	static MaterialDataManager* s_Instance;

	std::map<std::string, std::shared_ptr<const MaterialData>> m_Data;
};