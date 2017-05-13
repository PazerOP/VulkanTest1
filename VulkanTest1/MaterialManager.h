#pragma once
#include "DataStore.h"

class LogicalDevice;
class Material;

class MaterialManager final : public DataStore<MaterialManager, Material>
{
public:
	MaterialManager(LogicalDevice& device);

	void Reload() override;
};