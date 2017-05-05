#include "stdafx.h"
#include "Swapchain.h"

#include "GraphicsPipeline.h"
#include "LogicalDevice.h"
#include "PhysicalDeviceData.h"

Swapchain::Swapchain(const std::shared_ptr<LogicalDevice>& device)
{
	m_Data = device->GetData()->GetSwapChainData();
	m_Device = device;

	CreateSwapchain();
	CreateImageViews();
}

void Swapchain::CreateSwapchain()
{
	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.setSurface(*m_Data->GetWindowSurface());

	createInfo.setMinImageCount(m_Data->GetSurfaceCapabilities().minImageCount);
	createInfo.setImageFormat(m_Format = m_Data->ChooseBestSurfaceFormat().format);
	createInfo.setImageColorSpace(m_Data->ChooseBestSurfaceFormat().colorSpace);
	createInfo.setImageExtent(m_Extent = m_Data->ChooseBestExtent2D());
	createInfo.setImageArrayLayers(1);
	createInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

	const auto device = m_Device.lock();
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

	createInfo.setPreTransform(m_Data->GetSurfaceCapabilities().currentTransform);

	createInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);

	createInfo.setPresentMode(m_Data->ChooseBestPresentMode());
	createInfo.setClipped(true);

	m_Swapchain = device->GetDevice().createSwapchainKHR(createInfo);

	m_SwapchainImages = device->GetDevice().getSwapchainImagesKHR(m_Swapchain);
}

void Swapchain::CreateImageViews()
{
	for (auto& img : m_SwapchainImages)
	{
		vk::ImageViewCreateInfo createInfo;

		createInfo.setImage(img);

		createInfo.setViewType(vk::ImageViewType::e2D);
		createInfo.format = m_Data->ChooseBestSurfaceFormat().format;

		createInfo.components.r = vk::ComponentSwizzle::eIdentity;
		createInfo.components.g = vk::ComponentSwizzle::eIdentity;
		createInfo.components.b = vk::ComponentSwizzle::eIdentity;
		createInfo.components.a = vk::ComponentSwizzle::eIdentity;

		createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		m_SwapchainImageViews.push_back(std::shared_ptr<vk::ImageView>(
			new vk::ImageView(m_Device.lock()->GetDevice().createImageView(createInfo)),
			[this](vk::ImageView* iv) { m_Device.lock()->GetDevice().destroyImageView(*iv); }));
	}
}

void Swapchain::CreateFramebuffers(const std::shared_ptr<GraphicsPipeline>& pipeline)
{
	const auto& device = m_Device.lock();
	for (const auto& imgView : m_SwapchainImageViews)
	{
		vk::FramebufferCreateInfo createInfo;
		createInfo.setRenderPass(*Vulkan().GetGraphicsPipeline()->GetRenderPass());
		createInfo.setAttachmentCount(1);
		createInfo.setPAttachments(imgView.get());
		createInfo.setWidth(m_Extent.width);
		createInfo.setHeight(m_Extent.height);
		createInfo.setLayers(1);

		m_Framebuffers.push_back(std::shared_ptr<vk::Framebuffer>(
			new vk::Framebuffer(device->GetDevice().createFramebuffer(createInfo)),
			[device](vk::Framebuffer* fb) { device->GetDevice().destroyFramebuffer(*fb); delete fb; }
		));
	}

	return;
}