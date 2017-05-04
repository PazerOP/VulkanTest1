#pragma once
#include <set>
#include <vulkan\vulkan.hpp>
#include "VulkanEnumOperators.h"

VULKAN_ENUM_OPERATORS(VkDebugReportFlagBitsEXT, vk::DebugReportFlagBitsEXT);

class LogicalDevice;

class rkrp_vulkan_exception : public std::runtime_error
{
public:
	rkrp_vulkan_exception(const char* msg) : std::runtime_error(msg) {}
	rkrp_vulkan_exception(const std::string& msg) : std::runtime_error(msg) {}
};

class IVulkan
{
public:
	virtual ~IVulkan() = default;

	virtual void Init() = 0;
	virtual bool IsInitialized() const = 0;
	virtual void Shutdown() = 0;

	virtual vk::Instance& GetInstance() = 0;

	virtual const std::shared_ptr<LogicalDevice>& GetLogicalDevice() = 0;
};

extern IVulkan& Vulkan();