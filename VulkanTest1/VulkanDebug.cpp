#include "stdafx.h"
#include "VulkanDebug.h"

VKAPI_ATTR VkResult VKAPI_CALL vkDebugMarkerSetObjectNameEXT(
	VkDevice                                    device,
	VkDebugMarkerObjectNameInfoEXT*             pNameInfo)
{
	auto func = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(device, __FUNCTION__);
	assert(func);
	if (!func)
		return VkResult::VK_ERROR_EXTENSION_NOT_PRESENT;

	throw not_implemented_error(__FUNCTION__);
	//return func(device, pNameInfo);
}

void VulkanDebug::SetObjectName(vk::Device device, const std::string& name)
{
	vk::DebugMarkerObjectNameInfoEXT nameInfo;
	nameInfo.setObjectType(vk::DebugReportObjectTypeEXT::eDevice);
	nameInfo.setObject((uint64_t)(VkDevice)device);
	nameInfo.setPObjectName(name.c_str());

	device.debugMarkerSetObjectNameEXT(&nameInfo);
}
