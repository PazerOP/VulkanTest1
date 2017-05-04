#include "PhysicalDeviceData.h"
#include "StringTools.h"
#include "SwapChainData.h"

SwapchainData::SwapchainData(const std::shared_ptr<const PhysicalDeviceData>& deviceData, const std::shared_ptr<vk::SurfaceKHR>& windowSurface)
{
	m_WindowSurface = windowSurface;
	m_PhysicalDeviceData = deviceData;
	const auto& physicalDevice = deviceData->GetPhysicalDevice();

	m_AllSurfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*m_WindowSurface);
	m_AllSurfaceFormats = physicalDevice.getSurfaceFormatsKHR(*m_WindowSurface);
	m_AllPresentModes = physicalDevice.getSurfacePresentModesKHR(*m_WindowSurface);

	TestSuitability();
}

void SwapchainData::TestSuitability()
{
	if (m_PhysicalDeviceData.expired())
		throw std::runtime_error("PhysicalDeviceData weak_ptr was expired");

	const auto deviceData = m_PhysicalDeviceData.lock();

	if (m_AllSurfaceFormats.empty())
	{
		m_SuitabilityMessage = StringTools::CSFormat("unsuitable, no supported swap chain formats");
		m_Suitability = Suitability::NoSupportedSurfaceFormats;
		return;
	}

	m_SuitabilityMessage = StringTools::CSFormat("suitable");
	m_Suitability = Suitability::Suitable;
}
