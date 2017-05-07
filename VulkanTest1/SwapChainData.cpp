#include "stdafx.h"
#include "PhysicalDeviceData.h"
#include "StringTools.h"
#include "SwapChainData.h"
#include "Vulkan.h"

SwapchainData::SwapchainData(const std::shared_ptr<const PhysicalDeviceData>& deviceData, vk::SurfaceKHR& windowSurface)
{
	m_WindowSurface = &windowSurface;
	m_PhysicalDeviceData = deviceData;

	const auto& physicalDevice = deviceData->GetPhysicalDevice();
	m_AllSurfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*m_WindowSurface);
	m_AllSurfaceFormats = physicalDevice.getSurfaceFormatsKHR(*m_WindowSurface);
	m_AllPresentModes = physicalDevice.getSurfacePresentModesKHR(*m_WindowSurface);

	m_BestValues = std::make_shared<BestValues>();

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
	ChooseAndRateImageCount();

	m_SuitabilityMessage = StringTools::CSFormat("suitable, rating {0}", m_Rating);
	m_Suitability = Suitability::Suitable;
}

void SwapchainData::ChooseAndRateSurfaceFormat()
{
	if (GetSurfaceFormats().size() == 1 && GetSurfaceFormats().front().format == vk::Format::eUndefined)
	{
		// We can pick whatever we want
		m_BestValues->m_SurfaceFormat.format = vk::Format::eB8G8R8A8Unorm;
		m_BestValues->m_SurfaceFormat.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
	}

	m_BestValues->m_SurfaceFormat.format = vk::Format::eUndefined;
	for (const auto& format : GetSurfaceFormats())
	{
		if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			m_Rating += 10;
			m_BestValues->m_SurfaceFormat = format;
		}
	}

	if (m_BestValues->m_SurfaceFormat.format == vk::Format::eUndefined)
		throw rkrp_vulkan_exception(StringTools::CSFormat("Need a better way to handle this ({0})", __FUNCTION__));
}

void SwapchainData::ChooseAndRatePresentMode()
{
	const auto begin = std::begin(PRIORITIZED_SYNC_MODES);
	const auto end = std::end(PRIORITIZED_SYNC_MODES);

	size_t lowestIndex = std::distance(begin, std::find_if(begin, end, [](const auto& lhs) { return lhs.first == vk::PresentModeKHR::eFifo; }));
	for (const vk::PresentModeKHR mode : GetPresentModes())
	{
		const auto found = std::find_if(begin, end, [mode](const auto& lhs) { return lhs.first == mode; });
		assert(found != end);
		if (found == end)
			continue;

		m_Rating += found->second;

		const size_t index = std::distance(begin, found);

		lowestIndex = std::min(lowestIndex, index);
	}

	m_BestValues->m_PresentMode = PRIORITIZED_SYNC_MODES[lowestIndex].first;
}

void SwapchainData::ChooseAndRateExtent2D()
{
	const auto& surfaceCaps = GetSurfaceCapabilities();

	m_Rating += Remap(0, 5, 16384 * 16384, 1, surfaceCaps.minImageExtent.width * surfaceCaps.minImageExtent.height);
	m_Rating += Remap(0, 5, 1, 16384 * 16384, surfaceCaps.maxImageExtent.width * surfaceCaps.maxImageExtent.height);

	m_BestValues->m_Extent2D.width = std::clamp(surfaceCaps.currentExtent.width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
	m_BestValues->m_Extent2D.height = std::clamp(surfaceCaps.currentExtent.height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);
}

void SwapchainData::ChooseAndRateImageCount()
{
	const auto& surfaceCaps = GetSurfaceCapabilities();

	m_Rating += Remap(0, 5, 3, 2, (float)surfaceCaps.minImageCount);

	m_BestValues->m_ImageCount = surfaceCaps.maxImageCount ?
		std::min(surfaceCaps.minImageCount + 1, surfaceCaps.maxImageCount) :
		surfaceCaps.minImageCount + 1;
}
