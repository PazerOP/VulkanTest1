#include "stdafx.h"
#include "VulkanDebug.h"

std::map<uint64_t, std::string> VulkanDebug::s_DebugNames;

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

const std::string* VulkanDebug::GetObjectName(uint64_t objectID)
{
	auto found = s_DebugNames.find(objectID);
	if (found != s_DebugNames.end())
		return &(found->second);

	return nullptr;
}

void VulkanDebug::SetObjectName(const vk::Image& image, const std::string& name)
{
	SetObjectName((uint64_t)(VkImage)image, name);
}

void VulkanDebug::SetObjectName(uint64_t objectID, const std::string& name)
{
	s_DebugNames.insert_or_assign(objectID, name);
}