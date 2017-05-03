#include "Vulkan.h"

#include "FixedWindows.h"
#include "Log.h"
#include "Main.h"
#include "Util.h"

class _Vulkan final : public IVulkan
{
public:
	void Init() override;
	bool IsInitialized() const override { return !!m_Instance; }
	void Shutdown() override;

	vk::Instance& GetInstance() override;

	vk::Queue GetQueue(QueueType q) override;

private:
	void InitExtensions();
	void InitValidationLayers();
	void InitInstance();
	void CreateWindowSurface();
	void AutodetectPhysicalDevice();
	std::vector<size_t> FindPresentationQueueFamilies(const vk::PhysicalDevice& device) const;
	std::vector<std::pair<size_t, vk::QueueFamilyProperties>> FindQueueFamilies(const vk::PhysicalDevice& device, vk::QueueFlagBits queueFlags) const;
	void InitDevice();

	vk::Instance m_Instance;
	vk::PhysicalDevice m_PhysicalDevice;
	vk::Device m_LogicalDevice;
	vk::Queue m_Queues[underlying_value(QueueType::Count)];
	vk::SurfaceKHR m_WindowSurface;

	static constexpr const char TAG[] = "[VulkanImpl] ";

	enum class PhysicalDeviceSuitability
	{
		Suitable,

		NoSupport_SwapChain,

		MissingQueue_Graphics,
		MissingQueue_Presentation,

		MissingExtension_SwapChain,
	};
	PhysicalDeviceSuitability RatePhysicalDevice(const vk::PhysicalDevice& device, float& rating, const char*& detail) const;

	std::vector<vk::ExtensionProperties> GetAvailableInstanceExtensions();
	std::vector<vk::LayerProperties> GetAvailableInstanceLayers();
	std::vector<vk::PhysicalDevice> GetAvailablePhysicalDevices();

	std::set<std::string> m_EnabledInstanceExtensions;
	std::set<std::string> m_EnabledInstanceLayers;

	struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR m_Capabilities;
		std::vector<vk::SurfaceFormatKHR> m_Formats;
		std::vector<vk::PresentModeKHR> m_PresentModes;
	};
	SwapChainSupportDetails QuerySwapChainSupport(const vk::PhysicalDevice& device) const;

	VkDebugReportCallbackEXT m_DebugMsgCallbackHandle;
	void AttachDebugMsgCallback();
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
		uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);
};

IVulkan& Vulkan()
{
	static _Vulkan s_Vulkan;
	return s_Vulkan;
}

void _Vulkan::Init()
{
	InitExtensions();
	InitValidationLayers();

	InitInstance();

	AttachDebugMsgCallback();
	CreateWindowSurface();

	AutodetectPhysicalDevice();
	InitDevice();

	Log::BlockMsg("Completed initialization");
}

void _Vulkan::Shutdown()
{
	m_EnabledInstanceExtensions.clear();
	m_EnabledInstanceLayers.clear();
	m_Instance.destroy();
}

vk::Instance& _Vulkan::GetInstance()
{
	if (!IsInitialized())
		throw rkrp_vulkan_exception(StringTools::CSFormat("Attempted to call {0}() when IsInitialized() returned false", __FUNCTION__));

	return m_Instance;
}

void _Vulkan::InitExtensions()
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

void _Vulkan::InitValidationLayers()
{
	Log::TagMsg(TAG, "Initializing validation layers...");

	const auto& layers = GetAvailableInstanceLayers();

	m_EnabledInstanceLayers.insert("VK_LAYER_LUNARG_parameter_validation"s);
	m_EnabledInstanceLayers.insert("VK_LAYER_LUNARG_standard_validation"s);
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

void _Vulkan::InitInstance()
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

		m_Instance = vk::createInstance(info);
	}
}

void _Vulkan::CreateWindowSurface()
{
	Log::TagMsg(TAG, "Creating window surface...");

	vk::Win32SurfaceCreateInfoKHR createInfo;
	createInfo.setHwnd(Main().GetAppWindow().GetWindow());
	createInfo.setHinstance(Main().GetAppInstance());

	m_WindowSurface = m_Instance.createWin32SurfaceKHR(createInfo);
}

void _Vulkan::AutodetectPhysicalDevice()
{
	Log::TagMsg(TAG, "Autodetecting the best physical device...");

	const auto physicalDevices = GetAvailablePhysicalDevices();
	if (physicalDevices.empty())
		throw rkrp_vulkan_exception("Unable to find any physical devices supporting Vulkan.");

	float bestDeviceRating = std::numeric_limits<float>::min();
	vk::PhysicalDevice bestDevice;
	for (auto& device : physicalDevices)
	{
		const auto properties = device.getProperties();

		float deviceRating;

		const std::string baseMsg = StringTools::CSFormat("    Device {1} (id {2})", TAG, properties.deviceName, properties.deviceID);

		const char* detail = nullptr;
		switch (RatePhysicalDevice(device, deviceRating, detail))
		{
		case PhysicalDeviceSuitability::MissingQueue_Graphics:
			Log::TagMsg(TAG, "{0} unusable, no graphics queue ({1})", baseMsg, detail);
			break;

		case PhysicalDeviceSuitability::MissingQueue_Presentation:
			Log::TagMsg(TAG, "{0} unusable, no presentation queue ({1})", baseMsg, detail);
			break;

		case PhysicalDeviceSuitability::NoSupport_SwapChain:
		case PhysicalDeviceSuitability::MissingExtension_SwapChain:
			Log::TagMsg(TAG, "{0} unusable, no swap chain support ({1})", baseMsg, detail);
			break;

		case PhysicalDeviceSuitability::Suitable:
			Log::TagMsg(TAG, "{0} suitable, rating {1}", baseMsg, deviceRating);
			if (deviceRating > bestDeviceRating)
			{
				bestDevice = device;
				bestDeviceRating = deviceRating;
			}
			break;

		default:
			assert(!"Unknown PhysicalDeviceSuitability");
		}
	}

	if (!bestDevice)
		throw rkrp_vulkan_exception("Unable to find any suitable physical device.");

	m_PhysicalDevice = bestDevice;

	const auto bestDeviceProperties = m_PhysicalDevice.getProperties();
	Log::TagMsg(TAG, "Automatically chose \"best\" device: {0} (id {1})", bestDeviceProperties.deviceName, bestDeviceProperties.deviceID);
}

_Vulkan::PhysicalDeviceSuitability _Vulkan::RatePhysicalDevice(const vk::PhysicalDevice& device, float& rating, const char*& detail) const
{
	detail = nullptr;
	rating = 0;

	const auto properties = device.getProperties();
	//const auto features = device.getFeatures();
	//const auto queueFamilyProperties = device.getQueueFamilyProperties();

	// Dedicated GPUs are usually significantly better than onboard
	if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
		rating += 10;

	// Software rendering is aids
	if (properties.deviceType == vk::PhysicalDeviceType::eCpu)
		rating -= 10;

	// Bonuses for onboard memory
	{
		const auto memoryProperties = device.getMemoryProperties();
		std::vector<std::pair<uint32_t, vk::MemoryHeapFlagBits>> memoryTypes;

		for (size_t i = 0; i < memoryProperties.memoryHeapCount; i++)
		{
			const auto& heap = memoryProperties.memoryHeaps[i];
			const auto& heapFlags = heap.flags;

			float scalar;
			if (heapFlags & vk::MemoryHeapFlagBits::eDeviceLocal)
				scalar = 1;		// 1 point per GB of full speed, device local memory
			else
				scalar = 0.25;	// 0.25 points per GB of shared memory

			static constexpr float BYTES_TO_GB = 1.0f / 1024 / 1024 / 1024;

			rating += scalar * BYTES_TO_GB * heap.size;
		}
	}

	if (FindQueueFamilies(device, vk::QueueFlagBits::eGraphics).empty())
		return PhysicalDeviceSuitability::MissingQueue_Graphics;

	if (FindPresentationQueueFamilies(device).empty())
		return PhysicalDeviceSuitability::MissingQueue_Presentation;

	// Check extensions
	{
		const auto extensions = device.enumerateDeviceExtensionProperties();

		const auto HasExtension = [&extensions](const std::string_view& sv) -> bool
		{
			return std::find_if(extensions.begin(), extensions.end(),
				[&sv](const vk::ExtensionProperties& ex) { return !sv.compare(ex.extensionName); }) != extensions.end();
		};

		if (!HasExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
		{
			detail = "missing extension " VK_KHR_SWAPCHAIN_EXTENSION_NAME;
			return PhysicalDeviceSuitability::MissingExtension_SwapChain;
		}
	}

	// Check swap chain support
	{
		const auto swapChainSupport = QuerySwapChainSupport(device);

		if (swapChainSupport.m_Formats.empty())
		{
			detail = "no supported swap chain formats";
			return PhysicalDeviceSuitability::NoSupport_SwapChain;
		}

		assert(false);
	}

	return PhysicalDeviceSuitability::Suitable;
}

std::vector<size_t> _Vulkan::FindPresentationQueueFamilies(const vk::PhysicalDevice& device) const
{
	std::vector<size_t> retVal;

	const auto queueFamilyProperties = device.getQueueFamilyProperties();

	for (size_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		const auto& current = queueFamilyProperties[i];
		if (device.getSurfaceSupportKHR(i, m_WindowSurface))
			retVal.push_back(i);
	}

	return retVal;
}

std::vector<std::pair<size_t, vk::QueueFamilyProperties>> _Vulkan::FindQueueFamilies(const vk::PhysicalDevice& device, vk::QueueFlagBits queueFlags) const
{
	std::vector<std::pair<size_t, vk::QueueFamilyProperties>> retVal;

	const auto queueFamilyProperties = device.getQueueFamilyProperties();

	for (size_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		const auto& current = queueFamilyProperties[i];

		if (current.queueFlags & queueFlags)
			retVal.push_back(std::make_pair(i, current));
	}

	return retVal;
}

void _Vulkan::InitDevice()
{
	Log::TagMsg(TAG, "Creating logical device...");

	const uint32_t graphicsQueueFamily = FindQueueFamilies(m_PhysicalDevice, vk::QueueFlagBits::eGraphics).front().first;

	// Check if our presentation queue is the same as our graphics queue
	const uint32_t presentationQueueFamily = [this, &graphicsQueueFamily]() -> uint32_t
	{
		const auto presentationQueueFamilies = FindPresentationQueueFamilies(m_PhysicalDevice);
		for (size_t i = 0; i < presentationQueueFamilies.size(); i++)
		{
			if (presentationQueueFamilies[i] == graphicsQueueFamily)
				return i;
		}

		return presentationQueueFamilies.front();
	}();

	std::vector<vk::DeviceQueueCreateInfo> dqCreateInfos;

	const float queuePriority = 1;

	// Graphics queue
	uint32_t graphicsQueueIndex;
	{
		vk::DeviceQueueCreateInfo graphicsQueue;
		graphicsQueue.setQueueCount(1);
		graphicsQueue.setQueueFamilyIndex(graphicsQueueFamily);
		graphicsQueue.setPQueuePriorities(&queuePriority);
		dqCreateInfos.push_back(graphicsQueue);
		graphicsQueueIndex = dqCreateInfos.size() - 1;
	}

	// Presentation queue, might be the same as the graphics queue
	uint32_t presentationQueueIndex = graphicsQueueIndex;
	if (presentationQueueFamily != graphicsQueueFamily)
	{
		vk::DeviceQueueCreateInfo presentationQueue;
		presentationQueue.setQueueCount(1);
		presentationQueue.setQueueFamilyIndex(presentationQueueFamily);
		presentationQueue.setPQueuePriorities(&queuePriority);
		dqCreateInfos.push_back(presentationQueue);
		presentationQueueIndex = dqCreateInfos.size() - 1;
	}

	vk::PhysicalDeviceFeatures features;

	vk::DeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.setQueueCreateInfoCount(dqCreateInfos.size());
	deviceCreateInfo.setPQueueCreateInfos(dqCreateInfos.data());

	// Extensions
	static constexpr const char* DEVICE_EXTENSIONS[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};
	deviceCreateInfo.setPpEnabledExtensionNames(DEVICE_EXTENSIONS);
	deviceCreateInfo.setEnabledExtensionCount(sizeof(DEVICE_EXTENSIONS) / sizeof(DEVICE_EXTENSIONS[0]));

	m_LogicalDevice = m_PhysicalDevice.createDevice(deviceCreateInfo);

	m_Queues[underlying_value(QueueType::Graphics)] = m_LogicalDevice.getQueue(graphicsQueueFamily, graphicsQueueIndex);

	// Potentially shared
	if (presentationQueueIndex == graphicsQueueIndex)
		m_Queues[underlying_value(QueueType::Presentation)] = m_Queues[underlying_value(QueueType::Graphics)];
	else
		m_Queues[underlying_value(QueueType::Presentation)] = m_LogicalDevice.getQueue(presentationQueueFamily, presentationQueueIndex);
}

_Vulkan::SwapChainSupportDetails _Vulkan::QuerySwapChainSupport(const vk::PhysicalDevice& device) const
{
	SwapChainSupportDetails retVal;

	retVal.m_Capabilities = device.getSurfaceCapabilitiesKHR(m_WindowSurface);
	retVal.m_Formats = device.getSurfaceFormatsKHR(m_WindowSurface);
	retVal.m_PresentModes = device.getSurfacePresentModesKHR(m_WindowSurface);

	return retVal;
}

void _Vulkan::AttachDebugMsgCallback()
{
	Log::TagMsg(TAG, "Attaching debug msg callback...");

	auto func = (PFN_vkCreateDebugReportCallbackEXT)m_Instance.getProcAddr("vkCreateDebugReportCallbackEXT");
	assert(func);
	if (func != nullptr)
	{
		vk::DebugReportCallbackCreateInfoEXT createInfo;

		createInfo.flags |= vk::DebugReportFlagBitsEXT::eInformation;
		createInfo.flags |= vk::DebugReportFlagBitsEXT::eWarning;
		createInfo.flags |= vk::DebugReportFlagBitsEXT::ePerformanceWarning;
		createInfo.flags |= vk::DebugReportFlagBitsEXT::eError;
		//createInfo.flags |= vk::DebugReportFlagBitsEXT::eDebug;
		createInfo.setPfnCallback(&DebugCallback);

		VkResult result = func((VkInstance)m_Instance, &(VkDebugReportCallbackCreateInfoEXT&)createInfo, nullptr, &m_DebugMsgCallbackHandle);
		if (result != VK_SUCCESS)
			throw rkrp_vulkan_exception("Failed to attach debug callback!");
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL _Vulkan::DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData)
{
	const vk::DebugReportFlagBitsEXT flagBits = (vk::DebugReportFlagBitsEXT)flags;

	const char* msgType = "[VULKAN UNKNOWN]";
	if (!!(flagBits & vk::DebugReportFlagBitsEXT::eInformation))
		msgType = "[VULKAN INFO]";
	else if (!!(flagBits & vk::DebugReportFlagBitsEXT::eWarning))
		msgType = "[VULKAN WARN]";
	else if (!!(flagBits & vk::DebugReportFlagBitsEXT::ePerformanceWarning))
		msgType = "[VULKAN PERF]";
	else if (!!(flagBits & vk::DebugReportFlagBitsEXT::eError))
		msgType = "[VULKAN ERROR]";
	else if (!!(flagBits & vk::DebugReportFlagBitsEXT::eDebug))
		msgType = "[VULKAN DEBUG]";

	Log::Msg("{0} {2}", msgType, obj, msg);

	return VK_FALSE;
}

std::vector<vk::ExtensionProperties> _Vulkan::GetAvailableInstanceExtensions()
{
	uint32_t extensionCount = 0;
	vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<vk::ExtensionProperties> retVal;
	retVal.resize(extensionCount);

	vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, retVal.data());

	return retVal;
}

std::vector<vk::LayerProperties> _Vulkan::GetAvailableInstanceLayers()
{
	return vk::enumerateInstanceLayerProperties();
}

std::vector<vk::PhysicalDevice> _Vulkan::GetAvailablePhysicalDevices()
{
	return GetInstance().enumeratePhysicalDevices();
}

vk::Queue _Vulkan::GetQueue(QueueType q)
{
	if (underlying_value(q) < 0 ||
		underlying_value(q) >= underlying_value(QueueType::Count))
	{
		throw rkrp_vulkan_exception(StringTools::CSFormat("Invalid QueueType {0}", underlying_value(q)));
	}

	return m_Queues[underlying_value(q)];
}
