#include "stdafx.h"
#include "ShaderGroup.h"

ShaderGroup::ShaderGroup(const std::shared_ptr<const ShaderGroupData>& data, LogicalDevice& device) :
	m_Data(data), m_Device(device)
{
	for (const auto& shaderDef : m_Data->GetShaderDefinitions())
	{
		m_Modules[underlying_value(shaderDef.m_Type)] = std::make_shared<ShaderModule>(shaderDef.m_Path, shaderDef.m_Type, m_Device);
	}
}

std::shared_ptr<const ShaderModule> ShaderGroup::GetModule(ShaderType type) const
{
	assert(validate_enum_value(type));
	return m_Modules[underlying_value(type)];
}