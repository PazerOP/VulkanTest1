#pragma once
#include "SwapChainData.h"
#include "Util.h"

#include <optional>
#include <vector>

class PhysicalDeviceData : public std::enable_shared_from_this<PhysicalDeviceData>
{
public:
	static std::shared_ptr<PhysicalDeviceData> Create(const vk::PhysicalDevice& device, const vk::SurfaceKHR& windowSurface);

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
	Suitability GetSuitability() const;
	std::string GetSuitabilityMessage() const;

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

	const std::vector<const char*>& ChooseBestExtensionSet() const { return m_BestExtensionSet; }

	std::optional<std::pair<uint32_t, vk::QueueFamilyProperties>> ChooseBestQueue(bool presentation, vk::QueueFlags flags) const;
	std::optional<std::pair<uint32_t, vk::QueueFamilyProperties>> ChooseBestQueue(bool presentation) const;

	const vk::SurfaceKHR& GetWindowSurface() const { return m_WindowSurface; }

	void IncludeSwapchainRating(const SwapchainData& scData);

private:
	PhysicalDeviceData();
	void Init(const vk::PhysicalDevice& device, const vk::SurfaceKHR& windowSurface);

	void RateDeviceSuitability();
	void FindPresentationQueueFamilies();

	bool m_Init;

	float m_Rating;
	float m_SwapchainRating;
	Suitability m_Suitability;
	SwapchainData::Suitability m_SwapchainSuitability;

	std::string m_SuitabilityMessageBase;
	std::string m_SuitabilityMessageExtra;
	std::string m_SuitabilityMessageSwapchain;

	std::vector<vk::ExtensionProperties> m_SupportedExtensions;
	std::vector<vk::LayerProperties> m_SupportedLayers;

	vk::PhysicalDevice m_Device;
	vk::SurfaceKHR m_WindowSurface;

	vk::PhysicalDeviceProperties m_Properties;
	vk::PhysicalDeviceFeatures m_Features;
	std::vector<vk::QueueFamilyProperties> m_QueueFamilies;
	std::vector<uint32_t> m_PresentationQueueFamilies;

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