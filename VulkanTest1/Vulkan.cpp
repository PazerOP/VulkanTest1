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

#include <chrono>

class _Vulkan final : public IVulkan
{
public:
	void Init() override;
	bool IsInitialized() const override { return !!m_Instance; }
	void Shutdown() override;

	vk::Instance& GetInstance() override;

	const std::shared_ptr<LogicalDevice>& GetLogicalDevice() override;

	const std::shared_ptr<Swapchain>& GetSwapchain() override;

	const std::shared_ptr<GraphicsPipeline>& GetGraphicsPipeline() override;

private:
	void InitExtensions();
	void InitValidationLayers();
	void InitInstance();
	void CreateWindowSurface();
	void AutodetectPhysicalDevice();
	void InitDevice();
	void InitSwapChain();
	void InitGraphicsPipeline();
	void InitFramebuffers();
	void InitTestCommandBuffer();

	vk::Instance m_Instance;
	std::shared_ptr<const PhysicalDeviceData> m_PhysicalDeviceData;
	std::shared_ptr<LogicalDevice> m_LogicalDevice;
	std::shared_ptr<Swapchain> m_Swapchain;
	std::shared_ptr<GraphicsPipeline> m_GraphicsPipeline;
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

IVulkan& Vulkan()
{
	static _Vulkan s_Vulkan;
	return s_Vulkan;
}

void _Vulkan::Init()
{
	constexpr auto total = 12;
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

	AutodetectPhysicalDevice();
	updateTitle(progress++);

	InitDevice();
	updateTitle(progress++);

	InitSwapChain();
	updateTitle(progress++);

	InitGraphicsPipeline();
	updateTitle(progress++);

	InitFramebuffers();
	updateTitle(progress++);

	InitTestCommandBuffer();
	updateTitle(progress++);

	const auto endTime = std::chrono::high_resolution_clock::now();

	// temp: init game
	Game().InitGame();
	updateTitle(progress++);

	assert(progress == (total + 1));

	// Completed vulkan initialization
	Log::BlockMsg("Completed initialization ({0} steps) in {1} seconds.", total, std::chrono::duration<float>(endTime - startTime).count());
}

void _Vulkan::Shutdown()
{
	m_EnabledInstanceExtensions.clear();
	m_EnabledInstanceLayers.clear();
	m_Instance.destroy();
}

vk::Instance& _Vulkan::GetInstance()
{
	assert(m_Instance);
	return m_Instance;
}

const std::shared_ptr<LogicalDevice>& _Vulkan::GetLogicalDevice()
{
	assert(m_LogicalDevice);
	return m_LogicalDevice;
}

const std::shared_ptr<Swapchain>& _Vulkan::GetSwapchain()
{
	assert(m_Swapchain);
	return m_Swapchain;
}

const std::shared_ptr<GraphicsPipeline>& _Vulkan::GetGraphicsPipeline()
{
	assert(m_GraphicsPipeline);
	return m_GraphicsPipeline;
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

	m_WindowSurface = std::make_shared<vk::SurfaceKHR>(m_Instance.createWin32SurfaceKHR(createInfo));
}

void _Vulkan::AutodetectPhysicalDevice()
{
	Log::TagMsg(TAG, "Autodetecting the best physical device...");

	std::vector<std::shared_ptr<PhysicalDeviceData>> physicalDevices;
	for (const auto& rawDevice : m_Instance.enumeratePhysicalDevices())
		physicalDevices.push_back(PhysicalDeviceData::Create(rawDevice, m_WindowSurface));

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

	m_PhysicalDeviceData = physicalDevices.back();

	Log::TagMsg(TAG, "Automatically chose \"best\" device: {0}", m_PhysicalDeviceData->GetSuitabilityMessage());
}

void _Vulkan::InitDevice()
{
	Log::TagMsg(TAG, "Creating logical device...");

	m_LogicalDevice = std::make_shared<LogicalDevice>(m_PhysicalDeviceData);
}

void _Vulkan::InitSwapChain()
{
	Log::TagMsg(TAG, "Creating swap chain...");

	m_Swapchain = std::make_shared<Swapchain>(m_LogicalDevice);
}

void _Vulkan::InitGraphicsPipeline()
{
	Log::TagMsg(TAG, "Creating graphics pipeline...");

	auto createInfo = std::make_shared<GraphicsPipelineCreateInfo>(m_LogicalDevice, m_Swapchain);

	createInfo->SetShader(ShaderType::Vertex, std::make_shared<ShaderModule>("shaders/vert.spv", m_LogicalDevice));
	createInfo->SetShader(ShaderType::Pixel, std::make_shared<ShaderModule>("shaders/frag.spv", m_LogicalDevice));

	m_GraphicsPipeline = std::make_shared<GraphicsPipeline>(createInfo);
}

void _Vulkan::InitFramebuffers()
{
	Log::TagMsg(TAG, "Creating framebuffers...");

	((ISwapchain_VulkanFriends*)m_Swapchain.get())->CreateFramebuffers(m_GraphicsPipeline);
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