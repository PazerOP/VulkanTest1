#include "stdafx.h"
#include "Swapchain.h"

#include "GraphicsPipeline.h"
#include "LogicalDevice.h"
#include "PhysicalDeviceData.h"

Swapchain::Swapchain(const std::shared_ptr<LogicalDevice>& device)
{
	m_Data = device->GetData()->GetSwapChainData();
	m_Device = device;
	m_InitValues = m_Data->GetBestValues();

	CreateSwapchain();
	CreateImageViews();
}

void Swapchain::CreateSwapchain()
{
	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.setSurface(*m_Data->GetWindowSurface());

	createInfo.setMinImageCount(m_InitValues->m_ImageCount);
	createInfo.setImageFormat(m_InitValues->m_SurfaceFormat.format);
	createInfo.setImageColorSpace(m_InitValues->m_SurfaceFormat.colorSpace);
	createInfo.setImageExtent(m_InitValues->m_Extent2D);
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

	createInfo.setPresentMode(m_InitValues->m_PresentMode);
	createInfo.setClipped(true);

	m_Swapchain = device->Get().createSwapchainKHR(createInfo);

	m_SwapchainImages = device->Get().getSwapchainImagesKHR(m_Swapchain);
}

void Swapchain::CreateImageViews()
{
	for (auto& img : m_SwapchainImages)
	{
		vk::ImageViewCreateInfo createInfo;

		createInfo.setImage(img);

		createInfo.setViewType(vk::ImageViewType::e2D);
		createInfo.format = m_InitValues->m_SurfaceFormat.format;

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
			new vk::ImageView(m_Device.lock()->Get().createImageView(createInfo)),
			[this](vk::ImageView* iv) { m_Device.lock()->Get().destroyImageView(*iv); }));
	}
}

void Swapchain::CreateFramebuffers()
{
	const auto& device = m_Device.lock();
	for (const auto& imgView : m_SwapchainImageViews)
	{
		vk::FramebufferCreateInfo createInfo;
		createInfo.setRenderPass(*device->GetGraphicsPipeline()->GetRenderPass());
		createInfo.setAttachmentCount(1);
		createInfo.setPAttachments(imgView.get());
		createInfo.setWidth(GetInitValues()->m_Extent2D.width);
		createInfo.setHeight(GetInitValues()->m_Extent2D.height);
		createInfo.setLayers(1);

		m_Framebuffers.push_back(std::shared_ptr<vk::Framebuffer>(
			new vk::Framebuffer(device->Get().createFramebuffer(createInfo)),
			[device](vk::Framebuffer* fb) { device->Get().destroyFramebuffer(*fb); delete fb; }
		));
	}

	return;
}