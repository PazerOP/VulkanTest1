#include "stdafx.h"
#include "ShaderGroup.h"

#include "ShaderGroupData.h"
#include "ShaderModule.h"
#include "ShaderModuleData.h"
#include "ShaderModuleDataManager.h"

ShaderGroup::ShaderGroup(const std::shared_ptr<const ShaderGroupData>& data, LogicalDevice& device) :
	m_Data(data), m_Device(device)
{
	for (const auto& shaderDef : m_Data->GetShaderDefinitions())
	{
		m_Modules[Enums::value(shaderDef->m_ModuleData->GetType())] = std::make_shared<ShaderModule>(shaderDef->m_ModuleData, m_Device);
	}
}

std::shared_ptr<const ShaderModule> ShaderGroup::GetModule(ShaderType type) const
{
	assert(Enums::validate(type));
	return m_Modules[Enums::value(type)];
}
