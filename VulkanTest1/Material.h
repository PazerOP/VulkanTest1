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

	const MaterialData& GetData() const { return *m_Data; }
	const std::shared_ptr<const MaterialData>& GetDataPtr() const { return m_Data; }

	const GraphicsPipeline& GetPipeline() const { return m_GraphicsPipeline.value(); }
	GraphicsPipeline& GetPipeline() { return m_GraphicsPipeline.value(); }

private:
	void InitResources();

	void InitDescriptorSetLayout();
	void InitGraphicsPipeline();
	void InitDescriptorSet();

	LogicalDevice& m_Device;
	std::shared_ptr<const MaterialData> m_Data;

	struct ShaderResource
	{
		std::string m_DebugName;
		uint32_t m_BindingIndex;
		vk::ShaderStageFlags m_Stages;
		std::variant<std::shared_ptr<Texture>, bool, int, float> m_Data;
	};
	std::vector<ShaderResource> m_Resources;

	struct ShaderResourceData
	{
		using BindingIndex = uint32_t;
		std::map<BindingIndex, std::shared_ptr<Texture>> m_Textures;
		std::map<BindingIndex, std::variant<bool, int, float>> m_SpecConstants;
	};
	std::map<ShaderType, ShaderResourceData> m_Resources_;

	std::map<uint32_t, std::vector<vk::DescriptorSet>> GetDescriptorSets() const;

	GraphicsPipelineCreateInfo::Specializations SetupSpecializations() const;

	std::shared_ptr<DescriptorSetLayout> m_DescriptorSetLayout;
	std::shared_ptr<DescriptorSet> m_DescriptorSet;

	std::optional<GraphicsPipeline> m_GraphicsPipeline;
};