#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>

class PhysicalDeviceData;

class SwapChainData
{
public:
	SwapChainData(const std::shared_ptr<PhysicalDeviceData>& deviceData, const vk::SurfaceKHR& windowSurface);

	enum class Suitability
	{
		Suitable,

		MissingExtension,
		NoSupportedSurfaceFormats,
	};

	Suitability GetSuitability() const { return m_Suitability; }
	const std::string& GetSuitabilityMessage() const { return m_SuitabilityMessage; }

	const vk::SurfaceCapabilitiesKHR& GetSurfaceCapabilities() const { return m_SurfaceCapabilities; }
	const std::vector<vk::SurfaceFormatKHR>& GetSurfaceFormats() const { return m_SurfaceFormats; }
	const std::vector<vk::PresentModeKHR>& GetPresentModes() const { return m_PresentModes; }

private:
	void TestSuitability();

	Suitability m_Suitability;
	std::string m_SuitabilityMessage;

	vk::SurfaceKHR m_WindowSurface;

	std::weak_ptr<PhysicalDeviceData> m_PhysicalDeviceData;

	vk::SurfaceCapabilitiesKHR m_SurfaceCapabilities;
	std::vector<vk::SurfaceFormatKHR> m_SurfaceFormats;
	std::vector<vk::PresentModeKHR> m_PresentModes;
};