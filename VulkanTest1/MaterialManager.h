#pragma once
#include "DataStore.h"

class Material;

class MaterialManager final : public DataStore<MaterialManager, Material>
{
public:
	void Reload() override;
};