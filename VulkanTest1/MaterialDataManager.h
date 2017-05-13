#pragma once
#include "DataStore.h"

class LogicalDevice;
class MaterialData;

class MaterialDataManager : public DataStore<MaterialDataManager, const MaterialData>
{
public:
	MaterialDataManager(LogicalDevice& device);

	void Reload() override;
};