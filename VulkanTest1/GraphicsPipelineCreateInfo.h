#pragma once
#include "ShaderModule.h"

class LogicalDevice;
class Swapchain;

class GraphicsPipelineCreateInfo
{
public:
	GraphicsPipelineCreateInfo();
	GraphicsPipelineCreateInfo(LogicalDevice& device);

	const LogicalDevice& GetDevice() const { assert(m_Device); return *m_Device; }
	LogicalDevice& GetDevice() { assert(m_Device); return *m_Device; }

	const ShaderModule* GetShader(ShaderType type) const;
	ShaderModule* GetShader(ShaderType type) { return const_cast<ShaderModule*>(const_this(this)->GetShader(type)); }
	void SetShader(ShaderType type, const std::shared_ptr<ShaderModule>& shader);

private:
	std::shared_ptr<ShaderModule> m_Shaders[underlying_value(ShaderType::Count)];

	LogicalDevice* m_Device;
};