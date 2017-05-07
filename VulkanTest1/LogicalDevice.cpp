#include "stdafx.h"
#include "LogicalDevice.h"

#include "GraphicsPipeline.h"
#include "GraphicsPipelineCreateInfo.h"
#include "Log.h"
#include "Swapchain.h"

std::unique_ptr<LogicalDevice> LogicalDevice::Create(const std::shared_ptr<const PhysicalDeviceData>& physicalDevice)
{
	auto retVal = std::unique_ptr<LogicalDevice>(new LogicalDevice());
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
		submitInfo.setPWaitSemaphores(&m_ImageAvailableSemaphore.get());
		submitInfo.setPWaitDstStageMask(waitStages);

		submitInfo.setCommandBufferCount(1);
		submitInfo.setPCommandBuffers(&m_CommandBuffers[imageIndex].get());

		submitInfo.setSignalSemaphoreCount(1);
		submitInfo.setPSignalSemaphores(&m_RenderFinishedSemaphore.get());
	}

	GetQueue(QueueType::Graphics).submit(submitInfo, nullptr);

	vk::PresentInfoKHR presentInfo;
	vk::Result presentResult = vk::Result::eSuccess;
	{
		presentInfo.setWaitSemaphoreCount(1);
		presentInfo.setPWaitSemaphores(&m_RenderFinishedSemaphore.get());

		presentInfo.setSwapchainCount(1);
		presentInfo.setPSwapchains(&m_Swapchain->Get());

		presentInfo.setPImageIndices(&imageIndex);

		presentInfo.setPResults(&presentResult);
	}

	vk::Result mainPresentResult = GetQueue(QueueType::Presentation).presentKHR(presentInfo);
	assert(mainPresentResult == vk::Result::eSuccess);
	assert(presentResult == vk::Result::eSuccess);
}

void LogicalDevice::WindowResized()
{
	Log::TagMsg(TAG, "Window resized");
}

LogicalDevice::~LogicalDevice()
{
	Get().waitIdle();

	// Semaphores
	m_ImageAvailableSemaphore.reset();
	m_RenderFinishedSemaphore.reset();

	// Graphics pipeline and swap chain wrappers
	m_GraphicsPipeline.reset();
	m_Swapchain.reset();

	// Command buffers
	m_CommandBuffers.clear();

	// Command pool
	m_CommandPool.reset();

	// Device
	m_LogicalDevice.reset();
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

	m_LogicalDevice = m_PhysicalDevice->GetPhysicalDevice().createDeviceUnique(deviceCreateInfo);

	m_Queues[underlying_value(QueueType::Graphics)] = m_LogicalDevice->getQueue(GetQueueFamily(QueueType::Graphics), graphicsQueueIndex);
	m_Queues[underlying_value(QueueType::Presentation)] = m_LogicalDevice->getQueue(GetQueueFamily(QueueType::Presentation), presentationQueueIndex);
}

void LogicalDevice::InitSwapchain()
{
	Log::TagMsg(TAG, "Creating swap chain...");

	m_Swapchain = Swapchain::Create(*this);
}

void LogicalDevice::InitGraphicsPipeline()
{
	Log::TagMsg(TAG, "Creating graphics pipeline...");

	auto createInfo = std::make_shared<GraphicsPipelineCreateInfo>(*this);

	createInfo->SetShader(ShaderType::Vertex, ShaderModule::Create("shaders/vert.spv", *this));
	createInfo->SetShader(ShaderType::Pixel, ShaderModule::Create("shaders/frag.spv", *this));

	m_GraphicsPipeline = GraphicsPipeline::Create(createInfo);
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

	m_CommandPool = Get().createCommandPoolUnique(createInfo);
}

void LogicalDevice::InitCommandBuffers()
{
	Log::TagMsg(TAG, "Creating command buffers...");

	const auto& framebuffers = m_Swapchain->GetFramebuffers();

	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.setCommandPool(*m_CommandPool);
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
	allocInfo.setCommandBufferCount(framebuffers.size());

	assert(m_CommandBuffers.empty());
	m_CommandBuffers = Get().allocateCommandBuffersUnique(allocInfo);

	for (size_t i = 0; i < framebuffers.size(); i++)
	{
		const auto& cmdBuffer = m_CommandBuffers[i];
		const auto& framebuffer = framebuffers[i];

		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

		cmdBuffer->begin(beginInfo);

		// Not too sure about this one...
		{
			vk::RenderPassBeginInfo renderPassInfo;
			renderPassInfo.setRenderPass(m_GraphicsPipeline->GetRenderPass());
			renderPassInfo.setFramebuffer(framebuffer);
			renderPassInfo.renderArea.setExtent(m_Swapchain->GetInitValues().m_Extent2D);

			vk::ClearValue clearColor;
			clearColor.setColor(vk::ClearColorValue(std::array<float, 4>{ 0, 0, 0, 1 }));

			renderPassInfo.setClearValueCount(1);
			renderPassInfo.setPClearValues(&clearColor);

			cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

			cmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_GraphicsPipeline->GetPipeline());

			cmdBuffer->draw(3, 1, 0, 0);

			cmdBuffer->endRenderPass();
		}

		cmdBuffer->end();
	}

}

void LogicalDevice::InitSemaphores()
{
	vk::SemaphoreCreateInfo createInfo;

	m_ImageAvailableSemaphore = Get().createSemaphoreUnique(createInfo);
	m_RenderFinishedSemaphore = Get().createSemaphoreUnique(createInfo);
}

void LogicalDevice::RecreateSwapChain()
{
	Get().waitIdle();

	InitSwapchain();
	InitGraphicsPipeline();
	InitFramebuffers();
	InitCommandBuffers();
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
