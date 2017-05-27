#pragma once
#include "GraphicsPipeline.h"
#include "IMaterial.h"

#include <filesystem>
#include <map>
#include <unordered_set>

class DescriptorSet;
class DescriptorSetLayout;
class LogicalDevice;
class MaterialData;
class Texture;

struct LayoutBinding;
namespace std
{
	template<> struct hash<LayoutBinding>
	{
		size_t operator()(const LayoutBinding& x) const;
	};
}

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
	static constexpr char TAG[] = "[Material] ";

	void InitDescriptorSet();
	void InitGraphicsPipeline();

	LogicalDevice& m_Device;
	std::shared_ptr<const MaterialData> m_Data;

	struct LayoutBinding
	{
		std::string m_FullName;
		std::string m_FriendlyName;
		vk::DescriptorSetLayoutBinding m_Binding;
		std::optional<std::variant<std::shared_ptr<Buffer>, std::shared_ptr<Texture>>> m_Data;

		bool operator==(const LayoutBinding& rhs) const;

		struct hash
		{
			size_t operator()(const LayoutBinding& x) const;
		};
	};

	std::unordered_set<LayoutBinding, LayoutBinding::hash> m_Bindings;

	std::map<uint32_t, std::vector<vk::DescriptorSet>> GetDescriptorSets() const;

	GraphicsPipelineCreateInfo::Specializations SetupSpecializations() const;
	void SetupTexModeSpecConstants(GraphicsPipelineCreateInfo::Specializations& specializations) const;
	void SetupParamSpecConstants(GraphicsPipelineCreateInfo::Specializations& specializations) const;

	std::shared_ptr<DescriptorSetLayout> m_DescriptorSetLayout;
	std::shared_ptr<DescriptorSet> m_DescriptorSet;

	std::optional<GraphicsPipeline> m_GraphicsPipeline;
};