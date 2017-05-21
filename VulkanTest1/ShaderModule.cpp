#include "stdafx.h"
#include "ShaderModule.h"

#include "LogicalDevice.h"
#include "ShaderModuleData.h"
#include "Vulkan.h"

#include <spirv_cross.hpp>

ShaderModule::ShaderModule(const std::shared_ptr<const ShaderModuleData>& data, LogicalDevice& device) :
	m_Data(data),
	m_Device(device)
{
	Log::Msg<LogType::ObjectLifetime>(__FUNCSIG__);

	vk::ShaderModuleCreateInfo createInfo;

	createInfo.setCodeSize(m_Data->GetCodeBytes().size());
	createInfo.setPCode(m_Data->GetCodeBytes().data());

	m_Shader = device->createShaderModuleUnique(createInfo);
}