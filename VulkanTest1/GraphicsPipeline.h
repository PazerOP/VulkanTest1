#pragma once
#include "BaseException.h"
#include "Buffer.h"
#include "GraphicsPipelineCreateInfo.h"
#include "ShaderType.h"

#include <spirv_common.hpp>

#include <forward_list>
#include <functional>
#include <optional>
#include <variant>

class DescriptorSet;

class GraphicsPipeline
{
public:
	GraphicsPipeline(LogicalDevice& device, const std::shared_ptr<const GraphicsPipelineCreateInfo>& createInfo);

	const GraphicsPipelineCreateInfo& GetCreateInfo() const { return *m_CreateInfo; }

	LogicalDevice& GetDevice() const { return m_Device; }

	const vk::Pipeline GetPipeline() const { return m_Pipeline.get(); }
	vk::Pipeline GetPipeline() { return m_Pipeline.get(); }

	const vk::PipelineLayout GetPipelineLayout() const { return m_Layout.get(); }
	vk::PipelineLayout GetPipelineLayout() { return m_Layout.get(); }

	void RecreatePipeline();

	class Exception : public BaseException<>
	{
	public:
		Exception(const std::string& type, const std::string& msg) : BaseException(type, msg) { }
	};

	class ConflictingSpecConstTypeException : public Exception
	{
		using BaseType = spirv_cross::SPIRType::BaseType;
	public:
		ConflictingSpecConstTypeException(const std::type_info& inputType, BaseType outputType, uint32_t index, ShaderType type);

	private:
		static std::string GenerateWhat(const std::type_info& inputType, BaseType outputType, uint32_t index, ShaderType type);
	};

private:
	struct ShaderStageData
	{
		ShaderStageData() = default;
		ShaderStageData(const ShaderStageData& other) = delete;
		ShaderStageData(ShaderStageData&& other) = delete;
		ShaderStageData& operator=(const ShaderStageData&) = delete;
		ShaderStageData& operator=(ShaderStageData&&) = delete;

		struct SpecializationInfo
		{
			SpecializationInfo() = default;
			SpecializationInfo(const SpecializationInfo&) = delete;
			SpecializationInfo(SpecializationInfo&&) = delete;
			SpecializationInfo& operator=(const SpecializationInfo&) = delete;
			SpecializationInfo& operator=(SpecializationInfo&&) = delete;

			template<class T> void InsertData(const T& input, vk::SpecializationMapEntry& entry);

			vk::SpecializationInfo m_Info;
			std::vector<vk::SpecializationMapEntry> m_MapEntries;
			std::vector<std::variant<vk::Bool32, int, float>> m_Storage;
		};

		std::vector<vk::PipelineShaderStageCreateInfo> m_StageCreateInfos;
		std::forward_list<SpecializationInfo> m_SpecializationInfos;
	};

	void CreatePipeline();
	void GenerateShaderStageCreateInfos(ShaderStageData& data) const;

	LogicalDevice& m_Device;

	std::vector<vk::DescriptorSetLayout> GetDescriptorSetLayouts() const;

	std::shared_ptr<const GraphicsPipelineCreateInfo> m_CreateInfo;

	vk::UniquePipelineLayout m_Layout;
	vk::UniquePipeline m_Pipeline;
};