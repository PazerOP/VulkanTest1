#include "stdafx.h"
#include "Vulkan.h"

#include "FixedWindows.h"
#include "Game.h"
#include "GraphicsPipeline.h"
#include "Log.h"
#include "LogicalDevice.h"
#include "Main.h"
#include "PhysicalDeviceData.h"
#include "Swapchain.h"
#include "Util.h"
#include "VulkanDebug.h"

#include <chrono>

#pragma comment(lib, "vulkan-1.lib")

static VulkanInstance* s_VulkanInstance = nullptr;
VulkanInstance& Vulkan()
{
	assert(s_VulkanInstance);
	if (!s_VulkanInstance)
		throw std::runtime_error("Attempted to access the vulkan instance before it was created or after it was destroyed!");

	return *s_VulkanInstance;
}

VulkanInstance::VulkanInstance()
{
	assert(!s_VulkanInstance);
	if (s_VulkanInstance)
		throw std::runtime_error("Only 1 vulkan instance may exist at a time!");

	s_VulkanInstance = this;

	Init();
}

VulkanInstance::~VulkanInstance()
{
	// Logical device wrapper
	m_LogicalDevice.reset();

	// Window surface
	m_WindowSurface.reset();

	// Debug callback
	m_DebugMsgCallbackHandle.reset();

	// Vulkan instance
	m_Instance.reset();

	assert(s_VulkanInstance == this);
	s_VulkanInstance = nullptr;
}

void VulkanInstance::Init()
{
	constexpr auto total = 7;
	auto updateTitle = [&total](int current)
	{
		//Main().GetAppWindow().SetTitle(StringTools::CSFormat("{0}Initialization progress: {1}%", TAG, int(current / (float)total * 100)));
	};

	const auto startTime = std::chrono::high_resolution_clock::now();

	int progress = 0;
	updateTitle(progress++);

	InitExtensions();
	updateTitle(progress++);

	InitValidationLayers();
	updateTitle(progress++);

	InitInstance();
	updateTitle(progress++);

	AttachDebugMsgCallback();
	updateTitle(progress++);

	CreateWindowSurface();
	updateTitle(progress++);

	InitDevice();
	updateTitle(progress++);

	const auto endTime = std::chrono::high_resolution_clock::now();

	// temp: init game
	Game().InitGame();
	updateTitle(progress++);

	assert(progress == (total + 1));

	// Completed vulkan initialization
	Log::BlockMsg("Completed initialization ({0} steps) in {1} seconds.", total, std::chrono::duration<float>(endTime - startTime).count());
}

void VulkanInstance::InitExtensions()
{
	Log::TagMsg(TAG, "Initializing extensions...");

	const auto& extensions = GetAvailableInstanceExtensions();

	m_EnabledInstanceExtensions.insert("VK_KHR_surface"s);
	m_EnabledInstanceExtensions.insert("VK_KHR_win32_surface"s);
	m_EnabledInstanceExtensions.insert(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	auto enabledExtensions = m_EnabledInstanceExtensions;

	std::string blockMsg = StringTools::CSFormat("{0} supported Vulkan extensions ({1} enabled):\n", extensions.size(), enabledExtensions.size());
	for (const auto& extension : extensions)
	{
		if (enabledExtensions.find(extension.extensionName) != enabledExtensions.end())
		{
			blockMsg += StringTools::CSFormat("    ENABLED: {0} (v{1})\n", extension.extensionName, extension.specVersion);
			enabledExtensions.erase(extension.extensionName);
		}
		else
			blockMsg += StringTools::CSFormat("             {0} (v{1})\n", extension.extensionName, extension.specVersion);
	}

	if (enabledExtensions.size())
		blockMsg += "\n"sv;

	for (const auto& extension : enabledExtensions)
		blockMsg += StringTools::CSFormat("    ENABLED, not listed: {0}\n", extension);

	Log::TagMsg(TAG, blockMsg);
}

void VulkanInstance::InitValidationLayers()
{
	Log::TagMsg(TAG, "Initializing validation layers...");

	const auto& layers = GetAvailableInstanceLayers();

	m_EnabledInstanceLayers.insert("VK_LAYER_LUNARG_parameter_validation"s);
	m_EnabledInstanceLayers.insert("VK_LAYER_LUNARG_standard_validation"s);
	m_EnabledInstanceLayers.insert("VK_LAYER_LUNARG_core_validation"s);
	m_EnabledInstanceLayers.insert("VK_LAYER_LUNARG_swapchain"s);
	auto enabledLayers = m_EnabledInstanceLayers;

	std::string blockMsg = StringTools::CSFormat("{0} supported Vulkan instance layers ({1} enabled):\n", layers.size(), enabledLayers.size());
	for (const auto& layer : layers)
	{
		if (enabledLayers.find(layer.layerName) != enabledLayers.end())
		{
			blockMsg += StringTools::CSFormat("    ENABLED: {0} (spec v{1}, impl v{2})\n", layer.layerName, layer.specVersion, layer.implementationVersion);
			enabledLayers.erase(layer.layerName);
		}
		else
			blockMsg += StringTools::CSFormat("             {0} (spec v{1}, impl v{2})\n", layer.layerName, layer.specVersion, layer.implementationVersion);
	}

	if (enabledLayers.size())
		blockMsg += "\n"sv;

	for (const auto& extension : enabledLayers)
		blockMsg += StringTools::CSFormat("    ENABLED, not listed: {0}", extension);

	Log::TagMsg(TAG, blockMsg);
}

void VulkanInstance::InitInstance()
{
	Log::TagMsg(TAG, "Initializing instance...");

	vk::ApplicationInfo appInfo;
	{
		appInfo.setPEngineName("RKRP Engine 3 - Matt Test");
		appInfo.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));

		appInfo.setPApplicationName("RKRP Engine Test App - Matt");
		appInfo.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));

		appInfo.setApiVersion(VK_API_VERSION_1_0);
	}

	// Init instance
	{
		vk::InstanceCreateInfo info;

		info.setPApplicationInfo(&appInfo);

		std::vector<const char*> layers;
		for (const auto& layer : m_EnabledInstanceLayers)
			layers.push_back(layer.c_str());

		std::vector<const char*> extensions;
		for (const auto& extension : m_EnabledInstanceExtensions)
			extensions.push_back(extension.c_str());

		info.setEnabledLayerCount(layers.size());
		info.setPpEnabledLayerNames(layers.data());

		info.setEnabledExtensionCount(extensions.size());
		info.setPpEnabledExtensionNames(extensions.data());

		m_Instance = vk::createInstanceUnique(info);
	}
}

void VulkanInstance::CreateWindowSurface()
{
	Log::TagMsg(TAG, "Creating window surface...");

	vk::Win32SurfaceCreateInfoKHR createInfo;
	createInfo.setHwnd(Main().GetAppWindow().GetWindow());
	createInfo.setHinstance(Main().GetAppInstance());

	m_WindowSurface = m_Instance->createWin32SurfaceKHRUnique(createInfo);
}

void VulkanInstance::InitDevice()
{
	Log::TagMsg(TAG, "Autodetecting the best physical device...");

	std::vector<std::shared_ptr<PhysicalDeviceData>> physicalDevices;
	for (const auto& rawDevice : m_Instance->enumeratePhysicalDevices())
		physicalDevices.push_back(PhysicalDeviceData::Create(rawDevice, m_WindowSurface.get()));

	if (physicalDevices.empty())
		throw rkrp_vulkan_exception("Unable to find any physical devices supporting Vulkan.");

	for (size_t i = 0; i < physicalDevices.size(); i++)
	{
		const auto& current = physicalDevices[i];
		if (current->GetSuitability() != PhysicalDeviceData::Suitability::Suitable)
		{
			Log::TagMsg(TAG, "    {0}", current->GetSuitabilityMessage());
			physicalDevices.erase(physicalDevices.begin() + i--);
		}
	}

	if (physicalDevices.empty())
		throw rkrp_vulkan_exception("Unable to find any suitable physical device.");

	std::sort(physicalDevices.begin(), physicalDevices.end(),
		[](const auto& a, const auto& b) { return a->GetRating() < b->GetRating(); });

	const auto physicalDevice = physicalDevices.back();

	Log::TagMsg(TAG, "Creating logical device with \"best\" physical device \"{0}\"...", physicalDevice->GetSuitabilityMessage());

	m_LogicalDevice.emplace(physicalDevice);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)(void*)vkGetInstanceProcAddr(instance, __FUNCTION__);
	assert(func);
	if (!func)
		return VkResult::VK_ERROR_EXTENSION_NOT_PRESENT;

	return func(instance, pCreateInfo, pAllocator, pCallback);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)(void*)vkGetInstanceProcAddr(instance, __FUNCTION__);
	assert(func);
	if (!func)
		return;

	return func(instance, callback, pAllocator);
}

void VulkanInstance::AttachDebugMsgCallback()
{
	Log::TagMsg(TAG, "Attaching debug msg callback...");

	vk::DebugReportCallbackCreateInfoEXT createInfo;

	createInfo.flags |= vk::DebugReportFlagBitsEXT::eInformation;
	createInfo.flags |= vk::DebugReportFlagBitsEXT::eWarning;
	createInfo.flags |= vk::DebugReportFlagBitsEXT::ePerformanceWarning;
	createInfo.flags |= vk::DebugReportFlagBitsEXT::eError;
	//createInfo.flags |= vk::DebugReportFlagBitsEXT::eDebug;
	createInfo.setPfnCallback(&DebugCallback);

	m_DebugMsgCallbackHandle = m_Instance->createDebugReportCallbackEXTUnique(createInfo);
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstance::DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData)
{
	const vk::DebugReportFlagsEXT flagBits((vk::DebugReportFlagBitsEXT)flags);

	const char* msgType = "[VULKAN UNKNOWN]";
	if (flagBits & vk::DebugReportFlagBitsEXT::eInformation)
		msgType = "[VULKAN INFO]";
	else if (flagBits & vk::DebugReportFlagBitsEXT::eWarning)
		msgType = "[VULKAN WARN]";
	else if (flagBits & vk::DebugReportFlagBitsEXT::ePerformanceWarning)
		msgType = "[VULKAN PERF]";
	else if (flagBits & vk::DebugReportFlagBitsEXT::eError)
		msgType = "[VULKAN ERROR]";
	else if (flagBits & vk::DebugReportFlagBitsEXT::eDebug)
		msgType = "[VULKAN DEBUG]";

	const std::string* objName = VulkanDebug::GetObjectName(obj);
	if (objName)
		Log::Msg("{0} {2}", msgType, *objName, msg);
	else
		Log::Msg("{0} {1}", msgType, msg);

	assert(!(flagBits & vk::DebugReportFlagBitsEXT::eError));

	return VK_FALSE;
}

std::vector<vk::ExtensionProperties> VulkanInstance::GetAvailableInstanceExtensions()
{
	uint32_t extensionCount = 0;
	vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<vk::ExtensionProperties> retVal;
	retVal.resize(extensionCount);

	vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, retVal.data());

	return retVal;
}

std::vector<vk::LayerProperties> VulkanInstance::GetAvailableInstanceLayers()
{
	return vk::enumerateInstanceLayerProperties();
}