#pragma once
#include "GraphicsPipeline.h"
#include "IMaterial.h"

#include <filesystem>
#include <map>

class DescriptorSet;
class DescriptorSetLayout;
class LogicalDevice;
class MaterialData;
class Texture;

class Material : public IMaterial
{
public:
	Material(const std::shared_ptr<const MaterialData>& data, LogicalDevice& device);

	virtual void Bind(const vk::CommandBuffer& cmdBuf) const override;

	const GraphicsPipeline& GetPipeline() const { return m_GraphicsPipeline.value(); }
	GraphicsPipeline& GetPipeline() { return m_GraphicsPipeline.value(); }

private:
	void InitTextures();
	void InitDescriptorSetLayout();
	void InitGraphicsPipeline();
	void InitDescriptorSet();

	uint32_t AdjustTextureBinding(const std::string& paramName, uint32_t originalBinding) const;

	void LoadTexture(const std::string& paramName, const std::string& textureName);

	LogicalDevice& m_Device;
	std::shared_ptr<const MaterialData> m_Data;

	std::map<std::string, std::shared_ptr<Texture>> m_Textures;

	std::map<uint32_t, std::vector<vk::DescriptorSet>> GetDescriptorSets() const;

	std::shared_ptr<DescriptorSetLayout> m_DescriptorSetLayout;
	std::shared_ptr<DescriptorSet> m_DescriptorSet;

	std::optional<GraphicsPipeline> m_GraphicsPipeline;
};