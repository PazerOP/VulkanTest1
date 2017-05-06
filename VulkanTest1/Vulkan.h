#pragma once
#include <set>
#include <vulkan\vulkan.hpp>
#include "VulkanEnumOperators.h"

VULKAN_ENUM_OPERATORS(VkDebugReportFlagBitsEXT, vk::DebugReportFlagBitsEXT);

class GraphicsPipeline;
class LogicalDevice;
class Swapchain;

class rkrp_vulkan_exception : public std::runtime_error
{
public:
	rkrp_vulkan_exception(const char* msg) : std::runtime_error(msg) {}
	rkrp_vulkan_exception(const std::string& msg) : std::runtime_error(msg) {}
};

class _Vulkan final
{
public:
	_Vulkan();
	~_Vulkan();

	void Init();
	bool IsInitialized() const { return !!m_Instance; }
	void Shutdown();

	vk::Instance& GetInstance();

	const std::shared_ptr<LogicalDevice>& GetLogicalDevice();

private:
	void InitExtensions();
	void InitValidationLayers();
	void InitInstance();
	void CreateWindowSurface();
	void InitDevice();

	vk::Instance m_Instance;
	std::shared_ptr<LogicalDevice> m_LogicalDevice;
	std::shared_ptr<vk::SurfaceKHR> m_WindowSurface;

	static void SurfaceKHRDeleter(vk::SurfaceKHR* s) { Vulkan().GetInstance().destroySurfaceKHR(*s); delete s; }

	static constexpr const char TAG[] = "[VulkanImpl] ";

	std::vector<vk::ExtensionProperties> GetAvailableInstanceExtensions();
	std::vector<vk::LayerProperties> GetAvailableInstanceLayers();

	std::set<std::string> m_EnabledInstanceExtensions;
	std::set<std::string> m_EnabledInstanceLayers;

	VkDebugReportCallbackEXT m_DebugMsgCallbackHandle;
	void AttachDebugMsgCallback();
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
														uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);
};

extern _Vulkan& Vulkan();