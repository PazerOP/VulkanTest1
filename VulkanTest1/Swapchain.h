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

private:
	std::shared_ptr<const SwapchainData> m_Data;
	std::weak_ptr<LogicalDevice> m_Device;

	vk::SwapchainKHR m_Swapchain;
};