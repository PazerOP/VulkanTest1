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

class LogicalDevice : public std::enable_shared_from_this<LogicalDevice>
{
public:
	static std::shared_ptr<LogicalDevice> Create(const std::shared_ptr<const PhysicalDeviceData>& physicalDevice);

	const std::shared_ptr<const PhysicalDeviceData>& GetData() const { return m_PhysicalDevice; }

	const vk::Device& Get() const { return m_LogicalDevice; }
	vk::Device& Get() { return m_LogicalDevice; }

	const vk::Queue& GetQueue(QueueType q) const;
	vk::Queue& GetQueue(QueueType q) { return const_cast<vk::Queue&>(const_this(this)->GetQueue(q)); }

	uint32_t GetQueueFamily(QueueType q) const;

	const std::shared_ptr<const Swapchain> GetSwapchain() const { return m_Swapchain; }
	const std::shared_ptr<Swapchain>& GetSwapchain() { return m_Swapchain; }

	const std::shared_ptr<const GraphicsPipeline> GetGraphicsPipeline() const { return m_GraphicsPipeline; }
	const std::shared_ptr<GraphicsPipeline>& GetGraphicsPipeline() { return m_GraphicsPipeline; }

	void DrawFrame();

private:
	LogicalDevice() = default;
	void Init(const std::shared_ptr<const PhysicalDeviceData>& physicalDevice);

#if _MSC_VER == 1910
	std::weak_ptr<const LogicalDevice> weak_from_this() const { return shared_from_this(); }
	std::weak_ptr<LogicalDevice> weak_from_this() { return shared_from_this(); }
#endif

	void InitDevice();
	void InitSwapchain();
	void InitGraphicsPipeline();
	void InitFramebuffers();
	void InitCommandPool();
	void InitCommandBuffers();
	void InitSemaphores();

	static constexpr const char TAG[] = "[LogicalDevice] ";

	void ChooseQueueFamilies();

	std::shared_ptr<const PhysicalDeviceData> m_PhysicalDevice;
	vk::Device m_LogicalDevice;

	uint32_t m_QueueFamilies[underlying_value(QueueType::Count)];
	vk::Queue m_Queues[underlying_value(QueueType::Count)];

	std::shared_ptr<Swapchain> m_Swapchain;
	std::shared_ptr<GraphicsPipeline> m_GraphicsPipeline;
	std::shared_ptr<vk::CommandPool> m_CommandPool;
	std::vector<std::shared_ptr<vk::CommandBuffer>> m_CommandBuffers;

	std::shared_ptr<vk::Semaphore> m_ImageAvailableSemaphore;
	std::shared_ptr<vk::Semaphore> m_RenderFinishedSemaphore;
};