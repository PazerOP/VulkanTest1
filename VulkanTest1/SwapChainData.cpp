#include "stdafx.h"
#include "PhysicalDeviceData.h"
#include "StringTools.h"
#include "SwapChainData.h"
#include "Vulkan.h"

SwapchainData::SwapchainData(const std::shared_ptr<const PhysicalDeviceData>& deviceData, const std::shared_ptr<vk::SurfaceKHR>& windowSurface)
{
	m_WindowSurface = windowSurface;
	m_PhysicalDeviceData = deviceData;
	const auto& physicalDevice = deviceData->GetPhysicalDevice();

	m_AllSurfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*m_WindowSurface);
	m_AllSurfaceFormats = physicalDevice.getSurfaceFormatsKHR(*m_WindowSurface);
	m_AllPresentModes = physicalDevice.getSurfacePresentModesKHR(*m_WindowSurface);

	RateSuitability();
}

void SwapchainData::RateSuitability()
{
	if (m_PhysicalDeviceData.expired())
		throw std::runtime_error("PhysicalDeviceData weak_ptr was expired");

	//const auto deviceData = m_PhysicalDeviceData.lock();
	const auto& surfaceCaps = GetSurfaceCapabilities();

	m_Rating = 0;

	if (m_AllSurfaceFormats.empty())
	{
		m_SuitabilityMessage = StringTools::CSFormat("unsuitable, no supported swap chain formats");
		m_Suitability = Suitability::NoSupportedSurfaceFormats;
		return;
	}

	ChooseAndRateSurfaceFormat();
	ChooseAndRatePresentMode();
	ChooseAndRateExtent2D();

	m_SuitabilityMessage = StringTools::CSFormat("suitable, rating {0}", m_Rating);
	m_Suitability = Suitability::Suitable;
}

void SwapchainData::ChooseAndRateSurfaceFormat()
{
	if (GetSurfaceFormats().size() == 1 && GetSurfaceFormats().front().format == vk::Format::eUndefined)
	{
		// We can pick whatever we want
		m_BestSurfaceFormat.format = vk::Format::eB8G8R8A8Unorm;
		m_BestSurfaceFormat.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
	}

	m_BestSurfaceFormat.format = vk::Format::eUndefined;
	for (const auto& format : GetSurfaceFormats())
	{
		if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			m_Rating += 10;
			m_BestSurfaceFormat = format;
		}
	}

	if (m_BestSurfaceFormat.format == vk::Format::eUndefined)
		throw rkrp_vulkan_exception(StringTools::CSFormat("Need a better way to handle this ({0})", __FUNCTION__));
}

void SwapchainData::ChooseAndRatePresentMode()
{
	m_BestPresentMode = vk::PresentModeKHR::eFifo;	// vsync, support for this is required in vulkan
	for (const vk::PresentModeKHR mode : GetPresentModes())
	{
		switch (mode)
		{
		case vk::PresentModeKHR::eImmediate:
			{
				m_Rating += 5;	// No vsync

				if (m_BestPresentMode == vk::PresentModeKHR::eFifo)
					m_BestPresentMode = mode;

				break;
			}

		case vk::PresentModeKHR::eFifoRelaxed:
			{
				m_Rating += 10;	// Functionally identical to nvidia's adaptive vsync

				if (m_BestPresentMode == vk::PresentModeKHR::eFifo || m_BestPresentMode == vk::PresentModeKHR::eImmediate)
					m_BestPresentMode = mode;

				break;
			}
		case vk::PresentModeKHR::eMailbox:
			{
				m_Rating += 20;	// Functionally identical to nvidia's "fast" vsync

				m_BestPresentMode = mode;

				break;
			}
		}
	}
}

void SwapchainData::ChooseAndRateExtent2D()
{
	const auto& surfaceCaps = GetSurfaceCapabilities();

	m_Rating += Remap(0, 5, 16384 * 16384, 1, surfaceCaps.minImageExtent.width * surfaceCaps.minImageExtent.height);
	m_Rating += Remap(0, 5, 1, 16384 * 16384, surfaceCaps.maxImageExtent.width * surfaceCaps.maxImageExtent.height);

	m_BestExtent2D.width = std::clamp(surfaceCaps.currentExtent.width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
	m_BestExtent2D.height = std::clamp(surfaceCaps.currentExtent.height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);
}
