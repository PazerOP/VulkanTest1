#pragma once
#include "PhysicalDeviceData.h"
#include "Util.h"
#include "Vulkan.h"

#include <memory>

enum class QueueType
{
	Graphics,
	Presentation,

	Count,
};
__forceinline bool validate_enum_value(QueueType value)
{
	return underlying_value(value) >= 0 && underlying_value(value) < underlying_value(QueueType::Count);
}

class LogicalDevice
{
public:
	static std::unique_ptr<LogicalDevice> Create(const std::shared_ptr<PhysicalDeviceData>& physicalDevice);
	~LogicalDevice();

	const PhysicalDeviceData& GetData() const { assert(m_PhysicalDeviceData); return *m_PhysicalDeviceData; }

	const vk::Device* operator->() const { return m_LogicalDevice.operator->(); }
	//vk::Device* operator->() { return &m_LogicalDevice.get(); }

	const vk::Device Get() const { return m_LogicalDevice.get(); }
	vk::Device Get() { return m_LogicalDevice.get(); }

	const vk::Queue& GetQueue(QueueType q) const;
	vk::Queue& GetQueue(QueueType q) { return const_cast<vk::Queue&>(const_this(this)->GetQueue(q)); }

	uint32_t GetQueueFamily(QueueType q) const;

	const Swapchain& GetSwapchain() const { return *m_Swapchain; }
	Swapchain& GetSwapchain() { return *m_Swapchain; }

	const GraphicsPipeline& GetGraphicsPipeline() const { assert(m_GraphicsPipeline); return *m_GraphicsPipeline; }
	GraphicsPipeline& GetGraphicsPipeline() { assert(m_GraphicsPipeline); return *m_GraphicsPipeline; }

	void DrawFrame();

	void WindowResized();

private:
	LogicalDevice() = default;
	void Init(const std::shared_ptr<PhysicalDeviceData>& physicalDevice);

	void InitDevice();
	void InitSwapchain();
	void InitGraphicsPipeline();
	void InitFramebuffers();
	void InitCommandPool();
	void InitCommandBuffers();
	void InitSemaphores();

	void RecreateSwapchain();

	static constexpr const char TAG[] = "[LogicalDevice] ";

	void ChooseQueueFamilies();

	std::shared_ptr<PhysicalDeviceData> m_PhysicalDeviceData;
	vk::UniqueDevice m_LogicalDevice;

	uint32_t m_QueueFamilies[underlying_value(QueueType::Count)];
	vk::Queue m_Queues[underlying_value(QueueType::Count)];

	std::unique_ptr<Swapchain> m_Swapchain;
	std::unique_ptr<GraphicsPipeline> m_GraphicsPipeline;
	vk::UniqueCommandPool m_CommandPool;
	std::vector<vk::UniqueCommandBuffer> m_CommandBuffers;

	vk::UniqueSemaphore m_ImageAvailableSemaphore;
	vk::UniqueSemaphore m_RenderFinishedSemaphore;
};