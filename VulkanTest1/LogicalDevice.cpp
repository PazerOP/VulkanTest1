#include "stdafx.h"
#include "LogicalDevice.h"

#include "GraphicsPipeline.h"
#include "GraphicsPipelineCreateInfo.h"
#include "Log.h"
#include "Swapchain.h"

std::shared_ptr<LogicalDevice> LogicalDevice::Create(const std::shared_ptr<const PhysicalDeviceData>& physicalDevice)
{
	auto retVal = std::shared_ptr<LogicalDevice>(new LogicalDevice());
	retVal->Init(physicalDevice);
	return retVal;
}

const vk::Queue& LogicalDevice::GetQueue(QueueType q) const
{
	assert(validate_enum_value(q));
	return m_Queues[underlying_value(q)];
}

uint32_t LogicalDevice::GetQueueFamily(QueueType q) const
{
	assert(validate_enum_value(q));
	return m_QueueFamilies[underlying_value(q)];
}

void LogicalDevice::DrawFrame()
{
	const auto result = Get().acquireNextImageKHR(m_Swapchain->Get(), std::numeric_limits<uint64_t>::max(), *m_ImageAvailableSemaphore, nullptr);
	assert(result.result == vk::Result::eSuccess);
	const uint32_t imageIndex = result.value;

	vk::SubmitInfo submitInfo;
	const vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	{
		submitInfo.setWaitSemaphoreCount(1);
		submitInfo.setPWaitSemaphores(m_ImageAvailableSemaphore.get());
		submitInfo.setPWaitDstStageMask(waitStages);

		submitInfo.setCommandBufferCount(1);
		submitInfo.setPCommandBuffers(m_CommandBuffers[imageIndex].get());

		submitInfo.setSignalSemaphoreCount(1);
		submitInfo.setPSignalSemaphores(m_RenderFinishedSemaphore.get());
	}

	GetQueue(QueueType::Graphics).submit(submitInfo, nullptr);

	vk::PresentInfoKHR presentInfo;
	vk::Result presentResult = vk::Result::eSuccess;
	{
		presentInfo.setWaitSemaphoreCount(1);
		presentInfo.setPWaitSemaphores(m_RenderFinishedSemaphore.get());

		presentInfo.setSwapchainCount(1);
		presentInfo.setPSwapchains(&m_Swapchain->Get());

		presentInfo.setPImageIndices(&imageIndex);

		presentInfo.setPResults(&presentResult);
	}

	vk::Result mainPresentResult = GetQueue(QueueType::Presentation).presentKHR(presentInfo);
	assert(mainPresentResult == vk::Result::eSuccess);
	assert(presentResult == vk::Result::eSuccess);
}

void LogicalDevice::Init(const std::shared_ptr<const PhysicalDeviceData>& physicalDevice)
{
	m_PhysicalDevice = physicalDevice;

	ChooseQueueFamilies();

	InitDevice();
	InitSwapchain();
	InitGraphicsPipeline();
	InitFramebuffers();
	InitCommandPool();
	InitCommandBuffers();
	InitSemaphores();
}

void LogicalDevice::InitDevice()
{
	std::vector<vk::DeviceQueueCreateInfo> dqCreateInfos;

	const float queuePriority = 1;

	// Graphics queue
	uint32_t graphicsQueueIndex;
	{
		vk::DeviceQueueCreateInfo graphicsQueue;
		graphicsQueue.setQueueCount(1);
		graphicsQueue.setQueueFamilyIndex(GetQueueFamily(QueueType::Graphics));
		graphicsQueue.setPQueuePriorities(&queuePriority);
		dqCreateInfos.push_back(graphicsQueue);
		graphicsQueueIndex = dqCreateInfos.size() - 1;
	}

	// Presentation queue, might be the same as the graphics queue
	uint32_t presentationQueueIndex = graphicsQueueIndex;
	if (GetQueueFamily(QueueType::Graphics) != GetQueueFamily(QueueType::Presentation))
	{
		vk::DeviceQueueCreateInfo presentationQueue;
		presentationQueue.setQueueCount(1);
		presentationQueue.setQueueFamilyIndex(GetQueueFamily(QueueType::Presentation));
		presentationQueue.setPQueuePriorities(&queuePriority);
		dqCreateInfos.push_back(presentationQueue);
		presentationQueueIndex = dqCreateInfos.size() - 1;
	}

	vk::PhysicalDeviceFeatures features;

	vk::DeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.setQueueCreateInfoCount(dqCreateInfos.size());
	deviceCreateInfo.setPQueueCreateInfos(dqCreateInfos.data());

	// Extensions
	deviceCreateInfo.setPpEnabledExtensionNames(m_PhysicalDevice->ChooseBestExtensionSet().data());
	deviceCreateInfo.setEnabledExtensionCount(m_PhysicalDevice->ChooseBestExtensionSet().size());

	m_LogicalDevice = m_PhysicalDevice->GetPhysicalDevice().createDevice(deviceCreateInfo);

	m_Queues[underlying_value(QueueType::Graphics)] = m_LogicalDevice.getQueue(GetQueueFamily(QueueType::Graphics), graphicsQueueIndex);
	m_Queues[underlying_value(QueueType::Presentation)] = m_LogicalDevice.getQueue(GetQueueFamily(QueueType::Presentation), presentationQueueIndex);
}

void LogicalDevice::InitSwapchain()
{
	Log::TagMsg(TAG, "Creating swap chain...");

	m_Swapchain = std::make_shared<Swapchain>(shared_from_this());
}

void LogicalDevice::InitGraphicsPipeline()
{
	Log::TagMsg(TAG, "Creating graphics pipeline...");

	const auto shared_this = shared_from_this();

	auto createInfo = std::make_shared<GraphicsPipelineCreateInfo>(shared_this);

	createInfo->SetShader(ShaderType::Vertex, std::make_shared<ShaderModule>("shaders/vert.spv", shared_this));
	createInfo->SetShader(ShaderType::Pixel, std::make_shared<ShaderModule>("shaders/frag.spv", shared_this));

	m_GraphicsPipeline = std::make_shared<GraphicsPipeline>(createInfo);
}

void LogicalDevice::InitFramebuffers()
{
	Log::TagMsg(TAG, "Creating framebuffers...");

	((ISwapchain_LogicalDeviceFriends*)m_Swapchain.get())->CreateFramebuffers();
}

void LogicalDevice::InitCommandPool()
{
	Log::TagMsg(TAG, "Creating command pool...");

	vk::CommandPoolCreateInfo createInfo;
	createInfo.queueFamilyIndex = GetQueueFamily(QueueType::Graphics);

	const auto weak_this = weak_from_this();
	m_CommandPool = std::shared_ptr<vk::CommandPool>(
		new vk::CommandPool(m_LogicalDevice.createCommandPool(createInfo)),
		[weak_this](vk::CommandPool* cp) { weak_this.lock()->Get().destroyCommandPool(*cp); delete cp; }
	);
}

void LogicalDevice::InitCommandBuffers()
{
	Log::TagMsg(TAG, "Creating command buffers...");

	const auto& framebuffers = m_Swapchain->GetFramebuffers();

	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.setCommandPool(*m_CommandPool);
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
	allocInfo.setCommandBufferCount(framebuffers.size());

	const auto weak_this = weak_from_this();
	const auto& cmdPool = m_CommandPool;
	for (const auto& buffer : Get().allocateCommandBuffers(allocInfo))
	{
		m_CommandBuffers.push_back(std::shared_ptr<vk::CommandBuffer>(
			new vk::CommandBuffer(buffer),
			[weak_this, cmdPool](vk::CommandBuffer* cb) { weak_this.lock()->Get().freeCommandBuffers(*cmdPool, 1, cb); delete cb; }
		));
	}

	for (size_t i = 0; i < framebuffers.size(); i++)
	{
		const auto& cmdBuffer = m_CommandBuffers[i];
		const auto& framebuffer = m_Swapchain->GetFramebuffers()[i];

		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

		cmdBuffer->begin(beginInfo);

		// Not too sure about this one...
		{
			vk::RenderPassBeginInfo renderPassInfo;
			renderPassInfo.setRenderPass(*m_GraphicsPipeline->GetRenderPass());
			renderPassInfo.setFramebuffer(*framebuffer);
			renderPassInfo.renderArea.setExtent(m_Swapchain->GetInitValues()->m_Extent2D);

			vk::ClearValue clearColor;
			clearColor.setColor(vk::ClearColorValue(std::array<float, 4>{ 0, 0, 0, 1 }));

			renderPassInfo.setClearValueCount(1);
			renderPassInfo.setPClearValues(&clearColor);

			cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

			cmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_GraphicsPipeline->GetPipeline());

			cmdBuffer->draw(3, 1, 0, 0);

			cmdBuffer->endRenderPass();
		}

		cmdBuffer->end();
	}

}

void LogicalDevice::InitSemaphores()
{
	vk::SemaphoreCreateInfo createInfo;

	const auto weak_this = weak_from_this();
	m_ImageAvailableSemaphore = std::shared_ptr<vk::Semaphore>(
		new vk::Semaphore(Get().createSemaphore(createInfo)),
		[weak_this](vk::Semaphore* s) { weak_this.lock()->Get().destroySemaphore(*s); delete s; }
		);

	m_RenderFinishedSemaphore = std::shared_ptr<vk::Semaphore>(
		new vk::Semaphore(Get().createSemaphore(createInfo)),
		[weak_this](vk::Semaphore* s) { weak_this.lock()->Get().destroySemaphore(*s); delete s; }
	);
}

void LogicalDevice::ChooseQueueFamilies()
{
	// Try to use a single queue for everything
	const auto bestEverythingQueue = m_PhysicalDevice->ChooseBestQueue(true, vk::QueueFlagBits::eGraphics);
	if (bestEverythingQueue.has_value())
	{
		m_QueueFamilies[underlying_value(QueueType::Graphics)] = m_QueueFamilies[underlying_value(QueueType::Presentation)] = bestEverythingQueue->first;
	}
	else
	{
		// Separate queues
		m_QueueFamilies[underlying_value(QueueType::Graphics)] = m_PhysicalDevice->ChooseBestQueue(false, vk::QueueFlagBits::eGraphics)->first;
		m_QueueFamilies[underlying_value(QueueType::Presentation)] = m_PhysicalDevice->ChooseBestQueue(true)->first;
	}
}
