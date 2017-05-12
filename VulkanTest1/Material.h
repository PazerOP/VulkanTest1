#pragma once
#include "GraphicsPipeline.h"
#include "IMaterial.h"

#include <filesystem>

class MaterialData;

class Material : public IMaterial
{
public:
	Material(const std::shared_ptr<MaterialData>& data);

private:
	std::shared_ptr<const MaterialData> m_Data;

	std::optional<GraphicsPipeline> m_GraphicsPipeline;
};