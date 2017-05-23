#include "stdafx.h"
#include "ShaderGroup.h"

#include "ShaderGroupData.h"
#include "ShaderModule.h"
#include "ShaderModuleData.h"
#include "ShaderModuleDataManager.h"

ShaderGroup::ShaderGroup(const std::shared_ptr<const ShaderGroupData>& data, LogicalDevice& device) :
	m_Data(data), m_Device(device)
{
	for (const auto& shaderModuleData : m_Data->GetShaderModulesData())
	{
		if (!shaderModuleData)
			continue;

		m_Modules[Enums::value(shaderModuleData->GetType())] = std::make_shared<ShaderModule>(shaderModuleData, m_Device);
	}
}

std::shared_ptr<const ShaderModule> ShaderGroup::GetModulePtr(ShaderType type) const
{
	assert(Enums::validate(type));
	return m_Modules[Enums::value(type)];
}
