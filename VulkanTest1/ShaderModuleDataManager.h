#pragma once
#include "DataStore.h"

class ShaderModuleData;

class ShaderModuleDataManager : public DataStore<ShaderModuleDataManager, const ShaderModuleData>
{
public:
	ShaderModuleDataManager(LogicalDevice& device);

	void Reload() override;
};