#pragma once
#include "SwapchainData.h"

#include <vulkan/vulkan.hpp>

class GraphicsPipeline;
class LogicalDevice;

class ISwapchain_LogicalDeviceFriends
{
public:
	virtual ~ISwapchain_LogicalDeviceFriends() = default;

private:
	virtual void CreateFramebuffers() = 0;
	virtual void Recreate(const std::shared_ptr<const SwapchainData>& data) = 0;

	friend class LogicalDevice;
};

class Swapchain : public ISwapchain_LogicalDeviceFriends
{
public:
	Swapchain(const std::shared_ptr<const SwapchainData>& data, LogicalDevice& device);
	
	// Stuff relies on us not moving around
	Swapchain(const Swapchain& other) = delete;
	Swapchain(Swapchain&& other) = delete;

	const vk::SwapchainKHR* operator->() const { return m_Swapchain.operator->(); }
	//vk::SwapchainKHR* operator->() { return &m_Swapchain.get(); }

	const vk::SwapchainKHR Get() const { return m_Swapchain.get(); }
	vk::SwapchainKHR Get() { return m_Swapchain.get(); }

	const std::vector<vk::Image>& GetSwapchainImages() const { return m_SwapchainImages; }

	std::vector<vk::Framebuffer> GetFramebuffers() const;

	const std::shared_ptr<const SwapchainData>& GetData() const { return m_Data; }

	const SwapchainData::BestValues& GetInitValues() const { return *m_InitValues; }

	const LogicalDevice& GetDevice() const { return m_Device; }

private:
	void Init();

	void CreateSwapchain();
	void CreateImageViews();
	void CreateFramebuffers() override;

	void Recreate(const std::shared_ptr<const SwapchainData>& data) override;

	std::shared_ptr<const SwapchainData> m_Data;
	std::shared_ptr<const SwapchainData::BestValues> m_InitValues;
	LogicalDevice& m_Device;

	std::vector<vk::Image> m_SwapchainImages;
	std::vector<vk::UniqueImageView> m_SwapchainImageViews;
	std::vector<vk::UniqueFramebuffer> m_Framebuffers;

	vk::UniqueSwapchainKHR m_Swapchain;
};