#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>

class PhysicalDeviceData;

class SwapchainData
{
public:
	SwapchainData(const std::shared_ptr<const PhysicalDeviceData>& deviceData, const std::shared_ptr<vk::SurfaceKHR>& windowSurface);

	enum class Suitability
	{
		Suitable,

		MissingExtension,
		NoSupportedSurfaceFormats,
	};

	std::shared_ptr<const PhysicalDeviceData> GetPhysicalDeviceData() const;
	const std::shared_ptr<vk::SurfaceKHR> GetWindowSurface() const { return m_WindowSurface; }

	float GetRating() const { return m_Rating; }
	Suitability GetSuitability() const { return m_Suitability; }
	const std::string& GetSuitabilityMessage() const { return m_SuitabilityMessage; }

	const vk::SurfaceCapabilitiesKHR& GetAllSurfaceCapabilities() const { return m_AllSurfaceCapabilities; }
	const std::vector<vk::SurfaceFormatKHR>& GetAllSurfaceFormats() const { return m_AllSurfaceFormats; }
	const std::vector<vk::PresentModeKHR>& GetAllPresentModes() const { return m_AllPresentModes; }

	const vk::SurfaceFormatKHR& ChooseBestSurfaceFormat() const { return m_BestSurfaceFormat; }
	vk::PresentModeKHR ChooseBestPresentMode() const { return m_BestPresentMode; }
	const vk::Extent2D& ChooseBestExtent2D() const { return m_BestExtent2D; }

private:
	void TestSuitability();

	float m_Rating;
	Suitability m_Suitability;
	std::string m_SuitabilityMessage;

	std::shared_ptr<vk::SurfaceKHR> m_WindowSurface;

	std::weak_ptr<const PhysicalDeviceData> m_PhysicalDeviceData;

	vk::SurfaceFormatKHR m_BestSurfaceFormat;
	vk::PresentModeKHR m_BestPresentMode;
	vk::Extent2D m_BestExtent2D;

	vk::SurfaceCapabilitiesKHR m_AllSurfaceCapabilities;
	std::vector<vk::SurfaceFormatKHR> m_AllSurfaceFormats;
	std::vector<vk::PresentModeKHR> m_AllPresentModes;
};