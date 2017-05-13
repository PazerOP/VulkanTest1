#pragma once
#include "DataStore.h"

class LogicalDevice;
class ShaderGroup;

class ShaderGroupManager : public DataStore<ShaderGroupManager, ShaderGroup>
{
public:
	ShaderGroupManager(LogicalDevice& device);

	void Reload() override;
};