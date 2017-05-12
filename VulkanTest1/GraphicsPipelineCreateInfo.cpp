#include "stdafx.h"
#include "GraphicsPipelineCreateInfo.h"

#include "LogicalDevice.h"
#include "Vulkan.h"

GraphicsPipelineCreateInfo::GraphicsPipelineCreateInfo(LogicalDevice& device) :
	m_Device(device)
{
}