#pragma once

class LogicalDevice;
class ShaderModuleData;

class ShaderModule
{
public:
	ShaderModule(const std::shared_ptr<const ShaderModuleData>& data, LogicalDevice& device);

	const ShaderModuleData& GetData() const { return *m_Data; }
	const std::shared_ptr<const ShaderModuleData>& GetDataPtr() const { return m_Data; }

	const LogicalDevice& GetDevice() const { m_Device; }
	LogicalDevice& GetDevice() { m_Device; }

	const vk::ShaderModule* operator->() const { return m_Shader.operator->(); }
	//vk::ShaderModule* operator->() { return &m_Shader; }

	const vk::ShaderModule Get() const { return m_Shader.get(); }
	vk::ShaderModule Get() { return m_Shader.get(); }

private:
	vk::UniqueShaderModule m_Shader;
	LogicalDevice& m_Device;

	std::shared_ptr<const ShaderModuleData> m_Data;
};