#pragma once
#include "SwapchainData.h"

#include <vulkan/vulkan.hpp>

class LogicalDevice;

class Swapchain
{
public:
	Swapchain(const std::shared_ptr<LogicalDevice>& device);

	const vk::SwapchainKHR& GetSwapchain() const { return m_Swapchain; }
	vk::SwapchainKHR& GetSwapchain() { return m_Swapchain; }

	const std::vector<vk::Image>& GetSwapchainImages() const { return m_SwapchainImages; }

	const std::shared_ptr<const SwapchainData>& GetData() const { return m_Data; }

	const vk::Extent2D& GetExtent() const { return m_Extent; }

private:
	void CreateImageViews();

	vk::Extent2D m_Extent;

	std::shared_ptr<const SwapchainData> m_Data;
	std::weak_ptr<LogicalDevice> m_Device;

	std::vector<vk::Image> m_SwapchainImages;
	std::vector<std::shared_ptr<vk::ImageView>> m_SwapchainImageViews;

	vk::SwapchainKHR m_Swapchain;
};