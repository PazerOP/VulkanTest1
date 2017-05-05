#pragma once
#include "ShaderModule.h"

class LogicalDevice;

class GraphicsPipelineCreateInfo
{
public:
	GraphicsPipelineCreateInfo();
	GraphicsPipelineCreateInfo(const std::shared_ptr<LogicalDevice>& device, const std::shared_ptr<Swapchain>& swapchain);

	const std::shared_ptr<const LogicalDevice>& GetDevice() const { return m_Device; }
	const std::shared_ptr<LogicalDevice>& GetDevice() { return m_Device; }

	const std::shared_ptr<const Swapchain>& GetSwapchain() const { return m_Swapchain; }
	const std::shared_ptr<Swapchain>& GetSwapchain() { return m_Swapchain; }

	const std::shared_ptr<const ShaderModule> GetShader(ShaderType type) const;
	const std::shared_ptr<ShaderModule> GetShader(ShaderType type) { return std::const_pointer_cast<ShaderModule>(const_this(this)->GetShader(type)); }
	void SetShader(ShaderType type, const std::shared_ptr<ShaderModule>& shader);

private:
	std::shared_ptr<ShaderModule> m_Shaders[underlying_value(ShaderType::Count)];

	std::shared_ptr<LogicalDevice> m_Device;
	std::shared_ptr<Swapchain> m_Swapchain;
};