#pragma once
#include "GraphicsPipeline.h"
#include "MaterialDataManager.h"
#include "MaterialManager.h"
#include "PhysicalDeviceData.h"
#include "ShaderGroupManager.h"
#include "ShaderGroupDataManager.h"
#include "Swapchain.h"
#include "Util.h"

#include <memory>

enum class QueueType
{
	Graphics,
	Presentation,
	Transfer,

	Count,
};
__forceinline bool validate_enum_value(QueueType value)
{
	return underlying_value(value) >= 0 && underlying_value(value) < underlying_value(QueueType::Count);
}

class Mesh;
class Texture;

class LogicalDevice
{
public:
	LogicalDevice(const std::shared_ptr<PhysicalDeviceData>& physicalDevice);
	~LogicalDevice();

	const PhysicalDeviceData& GetData() const { assert(m_PhysicalDeviceData); return *m_PhysicalDeviceData; }

	const vk::Device* operator->() const { return m_LogicalDevice.operator->(); }
	vk::Device Get() const { return m_LogicalDevice.get(); }
	operator vk::Device() const { return Get(); }

	const vk::Queue& GetQueue(QueueType q) const;
	vk::Queue& GetQueue(QueueType q) { return const_cast<vk::Queue&>(const_this(this)->GetQueue(q)); }

	uint32_t GetQueueFamily(QueueType q) const;

	const Swapchain& GetSwapchain() const { return *m_Swapchain; }
	Swapchain& GetSwapchain() { return *m_Swapchain; }

	vk::RenderPass GetRenderPass() const { return m_RenderPass.get(); }

	void DrawFrame();

	void WindowResized();

	vk::UniqueCommandBuffer AllocCommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const;
	std::vector<vk::UniqueCommandBuffer> AllocCommandBuffers(uint32_t count, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const;
	void SubmitCommandBuffers(const vk::CommandBuffer& cmdBuf, QueueType q = QueueType::Graphics) const;
	void SubmitCommandBuffers(const std::initializer_list<vk::CommandBuffer>& cmdBufs, QueueType q = QueueType::Graphics) const;

	// a LogicalDevice should never be assigned, this is here to make
	// passing references everywhere to ourselves a safer prospect
	LogicalDevice& operator=(const LogicalDevice& rhs) = delete;
	LogicalDevice& operator=(LogicalDevice&& rhs) = delete;

private:
	void InitDevice();
	void InitSwapchain();
	void InitRenderPass();
	void InitFramebuffers();
	void InitCommandPool();
	void InitCommandBuffers();
	void InitSemaphores();

	void RecreateSwapchain();

	static constexpr const char TAG[] = "[LogicalDevice] ";

	void ChooseQueueFamilies();

	std::shared_ptr<Material> m_TestMaterial;
	std::unique_ptr<Mesh> m_TestVertexBuffer;

	std::shared_ptr<PhysicalDeviceData> m_PhysicalDeviceData;
	vk::UniqueDevice m_LogicalDevice;

	// These need to be initialized in a specific order
	std::optional<ShaderGroupManager> m_ShaderGroupManagerInstance;
	std::optional<ShaderGroupDataManager> m_ShaderGroupDataManagerInstance;
	std::optional<MaterialDataManager> m_MaterialDataManagerInstance;
	std::optional<MaterialManager> m_MaterialManagerInstance;

	uint32_t m_QueueFamilies[underlying_value(QueueType::Count)];
	vk::Queue m_Queues[underlying_value(QueueType::Count)];

	std::optional<Swapchain> m_Swapchain;
	vk::UniqueRenderPass m_RenderPass;
	//std::optional<GraphicsPipeline> m_GraphicsPipeline;
	vk::UniqueCommandPool m_CommandPool;
	std::vector<vk::UniqueCommandBuffer> m_CommandBuffers;

	vk::UniqueSemaphore m_ImageAvailableSemaphore;
	vk::UniqueSemaphore m_RenderFinishedSemaphore;
};