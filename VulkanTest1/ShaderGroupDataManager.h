#pragma once
#include "DataStore.h"

class LogicalDevice;
class ShaderGroupData;

class ShaderGroupDataManager : public DataStore<ShaderGroupDataManager, const ShaderGroupData>
{
public:
	ShaderGroupDataManager(LogicalDevice& device);

	void Reload() override;
};