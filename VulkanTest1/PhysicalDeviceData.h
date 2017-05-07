#pragma once
#include "SwapChainData.h"
#include "Util.h"

#include <optional>
#include <vector>

class PhysicalDeviceData : public std::enable_shared_from_this<PhysicalDeviceData>
{
public:
	static std::shared_ptr<PhysicalDeviceData> Create(const vk::PhysicalDevice& device, vk::SurfaceKHR& windowSurface);

	enum class Suitability
	{
		Suitable,

		MissingQueue_All,
		MissingQueue_Graphics,
		MissingQueue_Presentation,

		MissingRequiredExtension,

		SwapChain_Unsuitable,
	};


	float GetRating() const { return m_Rating; }
	Suitability GetSuitability() const { return m_Suitability; }
	const std::string& GetSuitabilityMessage() const { return m_SuitabilityMessage; }

	bool HasExtension(const std::string_view& name) const;
	const std::vector<vk::ExtensionProperties>& GetSupportedExtensions() const { return m_SupportedExtensions; }
	const std::vector<vk::LayerProperties>& GetSupportedLayers() const { return m_SupportedLayers; }

	vk::PhysicalDevice& GetPhysicalDevice() { return m_Device; }
	const vk::PhysicalDevice& GetPhysicalDevice() const { return m_Device; }

	const vk::PhysicalDeviceProperties& GetProperties() const { return m_Properties; }
	const vk::PhysicalDeviceFeatures& GetFeatures() const { return m_Features; }

	const std::vector<vk::QueueFamilyProperties>& GetQueueFamilies() const { return m_QueueFamilies; }
	std::vector<std::pair<uint32_t, vk::QueueFamilyProperties>> GetQueueFamilies(const vk::QueueFlags& queueFlags);
	const std::vector<uint32_t>& GetPresentationQueueFamilies() const { return m_PresentationQueueFamilies; }

	const std::shared_ptr<const SwapchainData> GetSwapChainData() const { return m_SwapChainData; }

	const std::vector<const char*>& ChooseBestExtensionSet() const { return m_BestExtensionSet; }

	std::optional<std::pair<uint32_t, vk::QueueFamilyProperties>> ChooseBestQueue(bool presentation, vk::QueueFlags flags) const;
	std::optional<std::pair<uint32_t, vk::QueueFamilyProperties>> ChooseBestQueue(bool presentation) const;

private:
	PhysicalDeviceData();
	void Init(const vk::PhysicalDevice& device, vk::SurfaceKHR& windowSurface);

	void RateDeviceSuitability(vk::SurfaceKHR& windowSurface);
	void FindPresentationQueueFamilies(const vk::SurfaceKHR& windowSurface);

	bool m_Init;

	float m_Rating;
	Suitability m_Suitability;
	std::string m_SuitabilityMessage;

	std::vector<vk::ExtensionProperties> m_SupportedExtensions;
	std::vector<vk::LayerProperties> m_SupportedLayers;

	vk::PhysicalDevice m_Device;

	vk::PhysicalDeviceProperties m_Properties;
	vk::PhysicalDeviceFeatures m_Features;
	std::vector<vk::QueueFamilyProperties> m_QueueFamilies;
	std::vector<uint32_t> m_PresentationQueueFamilies;

	std::shared_ptr<const SwapchainData> m_SwapChainData;

	static constexpr const char* REQUIRED_EXTENSIONS[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	// A list of optional extensions to enable, along with a weight of how important they are.
	static constexpr std::pair<const char*, float> OPTIONAL_EXTENSIONS[] =
	{
		{ "placeholder", 0.0f }
	};

	std::vector<const char*> m_BestExtensionSet;
};