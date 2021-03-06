#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>

class PhysicalDeviceData;

class SwapchainData
{
public:
	SwapchainData(const vk::PhysicalDevice& physical, const vk::SurfaceKHR& windowSurface);

	enum class Suitability
	{
		Suitable,

		MissingExtension,
		NoSupportedSurfaceFormats,
	};

	const vk::SurfaceKHR& GetWindowSurface() const { return m_WindowSurface; }

	float GetRating() const { return m_Rating; }
	Suitability GetSuitability() const { return m_Suitability; }
	const std::string& GetSuitabilityMessage() const { return m_SuitabilityMessage; }

	const vk::SurfaceCapabilitiesKHR& GetSurfaceCapabilities() const { return m_AllSurfaceCapabilities; }
	const std::vector<vk::SurfaceFormatKHR>& GetSurfaceFormats() const { return m_AllSurfaceFormats; }
	const std::vector<vk::PresentModeKHR>& GetPresentModes() const { return m_AllPresentModes; }

	struct BestValues
	{
		uint32_t m_ImageCount;
		vk::SurfaceFormatKHR m_SurfaceFormat;
		vk::PresentModeKHR m_PresentMode;
		vk::Extent2D m_Extent2D;
	};

	const std::shared_ptr<const BestValues> GetBestValues() const { return m_BestValues; }

private:
	void RateSuitability();

	void ChooseAndRateSurfaceFormat();
	void ChooseAndRatePresentMode();
	void ChooseAndRateExtent2D();
	void ChooseAndRateImageCount();

	static constexpr std::pair<vk::PresentModeKHR, float> PRIORITIZED_SYNC_MODES[] =
	{
		{ vk::PresentModeKHR::eFifoRelaxed, 10.0f },	// nvidia adaptive vsync
		{ vk::PresentModeKHR::eImmediate, 10.0f },		// no vsync
		{ vk::PresentModeKHR::eFifo, 0.0f },			// vsync

		// Effectively disabled (eFifo is guarenteed in vulkan)
		{ vk::PresentModeKHR::eMailbox, 20.0f },		// nvidia fast vsync
	};

	float m_Rating;
	Suitability m_Suitability;
	std::string m_SuitabilityMessage;

	vk::SurfaceKHR m_WindowSurface;

	std::shared_ptr<BestValues> m_BestValues;

	vk::SurfaceCapabilitiesKHR m_AllSurfaceCapabilities;
	std::vector<vk::SurfaceFormatKHR> m_AllSurfaceFormats;
	std::vector<vk::PresentModeKHR> m_AllPresentModes;
};