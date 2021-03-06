#include "stdafx.h"
#include "Swapchain.h"

#include "GraphicsPipeline.h"
#include "LogicalDevice.h"
#include "PhysicalDeviceData.h"

std::vector<vk::Framebuffer> Swapchain::GetFramebuffers() const
{
	std::vector<vk::Framebuffer> retVal;

	for (const auto& fb : m_Framebuffers)
		retVal.push_back(*fb);

	return retVal;
}

Swapchain::Swapchain(const std::shared_ptr<const SwapchainData>& data, LogicalDevice& device) :
	m_Data(data), m_Device(device)
{
	Log::Msg<LogType::ObjectLifetime>(__FUNCSIG__);
	Init();
}

void Swapchain::Init()
{
	m_InitValues = m_Data->GetBestValues();

	CreateSwapchain();
	CreateImageViews();
}

void Swapchain::CreateSwapchain()
{
	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.setSurface(m_Data->GetWindowSurface());

	createInfo.setMinImageCount(m_InitValues->m_ImageCount);
	createInfo.setImageFormat(m_InitValues->m_SurfaceFormat.format);
	createInfo.setImageColorSpace(m_InitValues->m_SurfaceFormat.colorSpace);
	createInfo.setImageExtent(m_InitValues->m_Extent2D);
	createInfo.setImageArrayLayers(1);
	createInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

	const uint32_t queueFamilyIndices[] = { GetDevice().GetQueueFamily(QueueType::Graphics), GetDevice().GetQueueFamily(QueueType::Presentation) };
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

	auto oldSwapchain = std::move(m_Swapchain);
	createInfo.setOldSwapchain(*oldSwapchain);

	auto device = GetDevice().Get();
	m_Swapchain = GetDevice()->createSwapchainKHRUnique(createInfo);

	m_SwapchainImages = device.getSwapchainImagesKHR(*m_Swapchain);
}

void Swapchain::CreateImageViews()
{
	m_SwapchainImageViews.clear();

	auto device = GetDevice().Get();
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

		m_SwapchainImageViews.push_back(device.createImageViewUnique(createInfo));
	}
}

void Swapchain::CreateFramebuffers()
{
	m_Framebuffers.clear();

	auto device = GetDevice().Get();
	for (const auto& imgView : m_SwapchainImageViews)
	{
		vk::FramebufferCreateInfo createInfo;
		createInfo.setRenderPass(GetDevice().GetRenderPass());
		createInfo.setAttachmentCount(1);

		const vk::ImageView attachments[] = { imgView.get() };
		createInfo.setPAttachments(attachments);
		createInfo.setWidth(GetInitValues().m_Extent2D.width);
		createInfo.setHeight(GetInitValues().m_Extent2D.height);
		createInfo.setLayers(1);

		m_Framebuffers.push_back(device.createFramebufferUnique(createInfo));
	}

	return;
}

void Swapchain::Recreate(const std::shared_ptr<const SwapchainData>& data)
{
	m_Data = data;
	Init();
}