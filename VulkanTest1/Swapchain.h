#pragma once
#include "SwapchainData.h"

#include <vulkan/vulkan.hpp>

class GraphicsPipeline;
class LogicalDevice;

class ISwapchain_LogicalDeviceFriends
{
public:
	virtual ~ISwapchain_LogicalDeviceFriends() = default;

	virtual void CreateFramebuffers() = 0;

	friend class LogicalDevice;
};

class Swapchain : public ISwapchain_LogicalDeviceFriends
{
public:
	Swapchain(const std::shared_ptr<LogicalDevice>& device);

	const vk::SwapchainKHR& Get() const { return m_Swapchain; }
	vk::SwapchainKHR& Get() { return m_Swapchain; }

	const std::vector<vk::Image>& GetSwapchainImages() const { return m_SwapchainImages; }

	const std::vector<std::shared_ptr<const vk::Framebuffer>>& GetFramebuffers() const { return reinterpret_cast<const std::vector<std::shared_ptr<const vk::Framebuffer>>&>(m_Framebuffers); }
	const std::vector<std::shared_ptr<vk::Framebuffer>>& GetFramebuffers() { return m_Framebuffers; }

	const std::shared_ptr<const SwapchainData>& GetData() const { return m_Data; }

	const std::shared_ptr<const SwapchainData::BestValues>& GetInitValues() const { return m_InitValues; }

private:
	void CreateSwapchain();
	void CreateImageViews();
	void CreateFramebuffers() override;

	std::shared_ptr<const SwapchainData> m_Data;
	std::shared_ptr<const SwapchainData::BestValues> m_InitValues;
	std::weak_ptr<LogicalDevice> m_Device;

	std::vector<vk::Image> m_SwapchainImages;
	std::vector<std::shared_ptr<vk::ImageView>> m_SwapchainImageViews;
	std::vector<std::shared_ptr<vk::Framebuffer>> m_Framebuffers;

	vk::SwapchainKHR m_Swapchain;
};