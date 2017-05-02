#include "Vulkan.h"

#include "Log.h"

class _Vulkan final : public IVulkan
{
public:
	void Init() override;
	bool IsInitialized() const override { return !!m_Instance; }
	void Shutdown() override;

	vk::Instance& GetInstance() override;

	std::vector<vk::ExtensionProperties> GetAvailableExtensions() override;
	std::set<std::string>& GetEnabledExtensions() override { return m_EnabledExtensions; }

	std::vector<vk::LayerProperties> GetAvailableInstanceLayers() override;
	std::set<std::string>& GetEnabledInstanceLayers() override { return m_EnabledInstanceLayers; }

	std::vector<vk::PhysicalDevice> GetAvailablePhysicalDevices() override;
	void SetPreferredPhysicalDevice(const vk::PhysicalDevice& device) override;

private:
	void InitExtensions();
	void InitValidationLayers();
	void InitInstance();
	void PostInitValidationLayers();
	void AutodetectPhysicalDevice();
	std::vector<std::pair<size_t, vk::QueueFamilyProperties>> FindQueueFamilies(const vk::PhysicalDevice& device, vk::QueueFlagBits queueFlags);
	void CreateLogicalDevice();

	vk::Instance m_Instance;
	vk::PhysicalDevice m_PreferredPhysicalDevice;
	vk::Device m_LogicalDevice;
	vk::Queue m_GraphicsQueue;

	std::set<std::string> m_EnabledExtensions;
	std::set<std::string> m_EnabledInstanceLayers;

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

	PostInitValidationLayers();

	AutodetectPhysicalDevice();
	CreateLogicalDevice();

	Log::BlockMsg("Vulkan: Completed initialization");
}

void _Vulkan::Shutdown()
{
	m_EnabledExtensions.clear();
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
	Log::Msg("Vulkan: Initializing extensions...");

	const auto& extensions = GetAvailableExtensions();

	GetEnabledExtensions().insert("VK_KHR_surface"s);
	GetEnabledExtensions().insert("VK_KHR_win32_surface"s);
	GetEnabledExtensions().insert(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	auto enabledExtensions = GetEnabledExtensions();

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

	Log::Msg(blockMsg);
}

void _Vulkan::InitValidationLayers()
{
	Log::Msg("Vulkan: Initializing validation layers...");

	const auto& layers = GetAvailableInstanceLayers();

	GetEnabledInstanceLayers().insert("VK_LAYER_LUNARG_parameter_validation"s);
	GetEnabledInstanceLayers().insert("VK_LAYER_LUNARG_standard_validation"s);
	auto enabledLayers = GetEnabledInstanceLayers();

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

Log::Msg(blockMsg);
}

void _Vulkan::InitInstance()
{
	Log::Msg("Vulkan: Initializing instance...");

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
		for (const auto& layer : GetEnabledInstanceLayers())
			layers.push_back(layer.c_str());

		std::vector<const char*> extensions;
		for (const auto& extension : GetEnabledExtensions())
			extensions.push_back(extension.c_str());

		info.setEnabledLayerCount(layers.size());
		info.setPpEnabledLayerNames(layers.data());

		info.setEnabledExtensionCount(extensions.size());
		info.setPpEnabledExtensionNames(extensions.data());

		m_Instance = vk::createInstance(info);
	}
}

void _Vulkan::PostInitValidationLayers()
{
	Log::Msg("Vulkan: Additional validation layer initialization...");

	AttachDebugMsgCallback();
}

void _Vulkan::AutodetectPhysicalDevice()
{
	std::string debugMsg;
	if (!m_PreferredPhysicalDevice)
	{
		Log::Msg("Vulkan: Autodetecting the best physical device...");

		const auto physicalDevices = GetAvailablePhysicalDevices();
		if (physicalDevices.empty())
			throw rkrp_vulkan_exception("Unable to find any physical devices supporting Vulkan.");

		float bestDeviceRanking = std::numeric_limits<float>::min();
		vk::PhysicalDevice bestDevice;
		for (auto& physicalDevice : physicalDevices)
		{
			const auto properties = physicalDevice.getProperties();
			const auto memoryProperties = physicalDevice.getMemoryProperties();
			const auto features = physicalDevice.getFeatures();
			const auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

			float deviceRanking = 0;

			// Just pick the first dedicated gpu for now
			if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
				deviceRanking += 1;

			if (FindQueueFamilies(physicalDevice, vk::QueueFlagBits::eGraphics).empty())
				continue;	// Completely unsuitable

			if (deviceRanking > bestDeviceRanking)
			{
				bestDevice = physicalDevice;
				bestDeviceRanking = deviceRanking;
			}
		}

		if (!bestDevice)
			throw rkrp_vulkan_exception("Unable to find any suitable physical device.");

		m_PreferredPhysicalDevice = bestDevice;

		debugMsg += "Automatically chose \"best\" device: ";
	}
	else
	{
		debugMsg += "User specified preferred device: ";
	}

	// Debug message about what device we chose
	{
		const auto properties = m_PreferredPhysicalDevice.getProperties();
		debugMsg += StringTools::CSFormat("{0} (id {1})", properties.deviceName, properties.deviceID);
	}

	Log::Msg(debugMsg);
}

std::vector<std::pair<size_t, vk::QueueFamilyProperties>> _Vulkan::FindQueueFamilies(const vk::PhysicalDevice& device, vk::QueueFlagBits queueFlags)
{
	std::vector<std::pair<size_t, vk::QueueFamilyProperties>> retVal;

	const auto queueFamilyProperties = device.getQueueFamilyProperties();

	for (size_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		if (queueFamilyProperties[i].queueFlags & queueFlags)
			retVal.push_back(std::make_pair(i, queueFamilyProperties[i]));
	}

	return retVal;
}

void _Vulkan::CreateLogicalDevice()
{
	Log::Msg("Vulkan: Creating logical device...");

	const auto queueFamilies = FindQueueFamilies(m_PreferredPhysicalDevice, vk::QueueFlagBits::eGraphics);

	vk::DeviceQueueCreateInfo dqCreateInfo;
	dqCreateInfo.setQueueCount(1);
	dqCreateInfo.setQueueFamilyIndex((uint32_t)queueFamilies.front().first);

	float queuePriority = 1;
	dqCreateInfo.setPQueuePriorities(&queuePriority);

	vk::PhysicalDeviceFeatures features;

	vk::DeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.setQueueCreateInfoCount(1);
	deviceCreateInfo.setPQueueCreateInfos(&dqCreateInfo);

	m_LogicalDevice = m_PreferredPhysicalDevice.createDevice(deviceCreateInfo);

	m_GraphicsQueue = m_LogicalDevice.getQueue((uint32_t)queueFamilies.front().first, 0);
}

void _Vulkan::AttachDebugMsgCallback()
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)m_Instance.getProcAddr("vkCreateDebugReportCallbackEXT");
	assert(func);
	if (func != nullptr)
	{
		vk::DebugReportCallbackCreateInfoEXT createInfo;

		createInfo.flags |= vk::DebugReportFlagBitsEXT::eInformation;
		createInfo.flags |= vk::DebugReportFlagBitsEXT::eWarning;
		createInfo.flags |= vk::DebugReportFlagBitsEXT::ePerformanceWarning;
		createInfo.flags |= vk::DebugReportFlagBitsEXT::eError;
		createInfo.flags |= vk::DebugReportFlagBitsEXT::eDebug;
		createInfo.setPfnCallback(&DebugCallback);

		VkResult result = func((VkInstance)m_Instance, &(VkDebugReportCallbackCreateInfoEXT&)createInfo, nullptr, &m_DebugMsgCallbackHandle);
		if (result != VK_SUCCESS)
			throw rkrp_vulkan_exception("Failed to attach debug callback!");
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL _Vulkan::DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData)
{
	const vk::DebugReportFlagBitsEXT flagBits = (vk::DebugReportFlagBitsEXT)flags;

	const char* msgType = "[UNKNOWN]";
	if (!!(flagBits & vk::DebugReportFlagBitsEXT::eInformation))
		msgType = "[INFO] ";
	else if (!!(flagBits & vk::DebugReportFlagBitsEXT::eWarning))
		msgType = "[WARN] ";
	else if (!!(flagBits & vk::DebugReportFlagBitsEXT::ePerformanceWarning))
		msgType = "[PERF] ";
	else if (!!(flagBits & vk::DebugReportFlagBitsEXT::eError))
		msgType = "[ERROR]";
	else if (!!(flagBits & vk::DebugReportFlagBitsEXT::eDebug))
		msgType = "[DEBUG]";

	Log::Msg("{0} validation layer: {2}", msgType, obj, msg);

	return VK_FALSE;
}

std::vector<vk::ExtensionProperties> _Vulkan::GetAvailableExtensions()
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

void _Vulkan::SetPreferredPhysicalDevice(const vk::PhysicalDevice& device)
{
	const std::vector<vk::PhysicalDevice> devices = GetAvailablePhysicalDevices();
	assert(std::find(devices.begin(), devices.end(), device) != devices.end());

	m_PreferredPhysicalDevice = device;
}
