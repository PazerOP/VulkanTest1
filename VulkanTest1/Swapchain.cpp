#include "Swapchain.h"

#include "LogicalDevice.h"
#include "PhysicalDeviceData.h"

Swapchain::Swapchain(const std::shared_ptr<LogicalDevice>& device)
{
	m_Data = device->GetData()->GetSwapChainData();
	m_Device = device;

	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.setSurface(*m_Data->GetWindowSurface());

	createInfo.setMinImageCount(m_Data->GetAllSurfaceCapabilities().minImageCount);
	createInfo.setImageFormat(m_Data->ChooseBestSurfaceFormat().format);
	createInfo.setImageColorSpace(m_Data->ChooseBestSurfaceFormat().colorSpace);
	createInfo.setImageExtent(m_Data->ChooseBestExtent2D());
	createInfo.setImageArrayLayers(1);
	createInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

	const uint32_t queueFamilyIndices[] = { device->GetQueueFamily(QueueType::Graphics), device->GetQueueFamily(QueueType::Presentation) };
	if (queueFamilyIndices[0] != queueFamilyIndices[1])
	{
		createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);

		createInfo.setQueueFamilyIndexCount(2);
		createInfo.setPQueueFamilyIndices(queueFamilyIndices);
	}
	else
	{
		createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
	}

	createInfo.setPreTransform(m_Data->GetAllSurfaceCapabilities().currentTransform);

	createInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);

	createInfo.setPresentMode(m_Data->ChooseBestPresentMode());
	createInfo.setClipped(true);

	m_Swapchain = device->GetDevice().createSwapchainKHR(createInfo);
}
