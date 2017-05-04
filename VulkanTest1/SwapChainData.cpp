#include "PhysicalDeviceData.h"
#include "StringTools.h"
#include "SwapChainData.h"

SwapChainData::SwapChainData(const std::shared_ptr<PhysicalDeviceData>& deviceData, const vk::SurfaceKHR& windowSurface)
{
	m_WindowSurface = windowSurface;
	m_PhysicalDeviceData = deviceData;
	const auto& physicalDevice = deviceData->GetPhysicalDevice();

	m_SurfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(m_WindowSurface);
	m_SurfaceFormats = physicalDevice.getSurfaceFormatsKHR(m_WindowSurface);
	m_PresentModes = physicalDevice.getSurfacePresentModesKHR(m_WindowSurface);

	TestSuitability();
}

void SwapChainData::TestSuitability()
{
	if (m_PhysicalDeviceData.expired())
		throw std::runtime_error("PhysicalDeviceData weak_ptr was expired");

	const auto deviceData = m_PhysicalDeviceData.lock();

	if (m_SurfaceFormats.empty())
	{
		m_SuitabilityMessage = StringTools::CSFormat("unsuitable, no supported swap chain formats");
		m_Suitability = Suitability::NoSupportedSurfaceFormats;
		return;
	}

	m_SuitabilityMessage = StringTools::CSFormat("suitable");
	m_Suitability = Suitability::Suitable;
}
