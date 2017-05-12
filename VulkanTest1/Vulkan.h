#pragma once
#include "LogicalDevice.h"

#include <optional>
#include <set>
#include <vulkan\vulkan.hpp>

class GraphicsPipeline;
class Swapchain;

class rkrp_vulkan_exception : public std::runtime_error
{
public:
	rkrp_vulkan_exception(const char* msg) : std::runtime_error(msg) {}
	rkrp_vulkan_exception(const std::string& msg) : std::runtime_error(msg) {}
};

class VulkanInstance final
{
public:
	VulkanInstance();
	~VulkanInstance();

	bool IsInitialized() const { return !!m_Instance; }

	const vk::Instance* operator->() const { return m_Instance.operator->(); }
	//vk::Instance* operator->() { return &m_Instance; }

	vk::Instance Get() { return m_Instance.get(); }

	LogicalDevice& GetLogicalDevice() { return *m_LogicalDevice; }

private:
	void Init();

	void InitExtensions();
	void InitValidationLayers();
	void InitInstance();
	void CreateWindowSurface();
	void InitDevice();

	vk::UniqueInstance m_Instance;
	std::optional<LogicalDevice> m_LogicalDevice;
	vk::UniqueSurfaceKHR m_WindowSurface;

	static constexpr const char TAG[] = "[VulkanImpl] ";

	std::vector<vk::ExtensionProperties> GetAvailableInstanceExtensions();
	std::vector<vk::LayerProperties> GetAvailableInstanceLayers();

	std::set<std::string> m_EnabledInstanceExtensions;
	std::set<std::string> m_EnabledInstanceLayers;

	vk::UniqueDebugReportCallbackEXT m_DebugMsgCallbackHandle;
	void AttachDebugMsgCallback();
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
														uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);
};

extern VulkanInstance& Vulkan();