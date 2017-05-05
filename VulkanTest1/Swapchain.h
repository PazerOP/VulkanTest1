#pragma once
#include "SwapchainData.h"

#include <vulkan/vulkan.hpp>

class GraphicsPipeline;
class LogicalDevice;

class ISwapchain_VulkanFriends
{
public:
	virtual ~ISwapchain_VulkanFriends() = default;

	virtual void CreateFramebuffers(const std::shared_ptr<GraphicsPipeline>& pipeline) = 0;

	friend class _Vulkan;
};

class Swapchain : public ISwapchain_VulkanFriends
{
public:
	Swapchain(const std::shared_ptr<LogicalDevice>& device);

	const vk::SwapchainKHR& GetSwapchain() const { return m_Swapchain; }
	vk::SwapchainKHR& GetSwapchain() { return m_Swapchain; }

	const std::vector<vk::Image>& GetSwapchainImages() const { return m_SwapchainImages; }

	const std::vector<std::shared_ptr<const vk::Framebuffer>>& GetFramebuffers() const { return reinterpret_cast<const std::vector<std::shared_ptr<const vk::Framebuffer>>&>(m_Framebuffers); }
	const std::vector<std::shared_ptr<vk::Framebuffer>>& GetFramebuffers() { return m_Framebuffers; }

	const std::shared_ptr<const SwapchainData>& GetData() const { return m_Data; }

	const vk::Extent2D& GetExtent() const { return m_Extent; }
	vk::Format GetFormat() const { return m_Format; }

private:
	void CreateSwapchain();
	void CreateImageViews();
	void CreateFramebuffers(const std::shared_ptr<GraphicsPipeline>& pipeline) override;

	vk::Extent2D m_Extent;
	vk::Format m_Format;

	std::shared_ptr<const SwapchainData> m_Data;
	std::weak_ptr<LogicalDevice> m_Device;

	std::vector<vk::Image> m_SwapchainImages;
	std::vector<std::shared_ptr<vk::ImageView>> m_SwapchainImageViews;
	std::vector<std::shared_ptr<vk::Framebuffer>> m_Framebuffers;

	vk::SwapchainKHR m_Swapchain;
};