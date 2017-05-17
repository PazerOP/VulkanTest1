#pragma once
#include "ShaderType.h"

class LogicalDevice;
class ShaderGroupData;
class ShaderModule;

// A group of shader modules to be used together, for example a vertex+pixel shader combo.
class ShaderGroup
{
public:
	ShaderGroup(const std::shared_ptr<const ShaderGroupData>& data, LogicalDevice& device);

	std::shared_ptr<const ShaderModule> GetModule(ShaderType type) const;
	std::shared_ptr<ShaderModule> GetModule(ShaderType type) { return std::const_pointer_cast<ShaderModule>(std::as_const(*this).GetModule(type)); }

	const std::shared_ptr<const ShaderGroupData>& GetData() const { return m_Data; }

	const LogicalDevice& GetDevice() const { return m_Device; }
	LogicalDevice& GetDevice() { return m_Device; }

private:
	LogicalDevice& m_Device;
	std::shared_ptr<const ShaderGroupData> m_Data;
	std::shared_ptr<ShaderModule> m_Modules[Enums::count<ShaderType>()];
};