#pragma once

class LogicalDevice;
class ShaderGroup;

class GraphicsPipelineCreateInfo
{
public:
	GraphicsPipelineCreateInfo(LogicalDevice& device);

	const LogicalDevice& GetDevice() const { return m_Device; }
	LogicalDevice& GetDevice() { return m_Device; }

	const std::shared_ptr<ShaderGroup>& GetShaderGroup() const { return m_ShaderGroup; }
	void SetShaderGroup(const std::shared_ptr<ShaderGroup>& shaderGroup) { m_ShaderGroup = shaderGroup; }

private:
	std::shared_ptr<ShaderGroup> m_ShaderGroup;

	LogicalDevice& m_Device;
};