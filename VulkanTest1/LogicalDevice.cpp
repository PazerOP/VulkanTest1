#include "stdafx.h"
#include "LogicalDevice.h"

#include "GraphicsPipeline.h"
#include "GraphicsPipelineCreateInfo.h"
#include "Log.h"
#include "Material.h"
#include "Mesh.h"
#include "ShaderGroupManager.h"
#include "Swapchain.h"
#include "Texture.h"

const vk::Queue& LogicalDevice::GetQueue(QueueType q) const
{
	assert(Enums::validate(q));
	return m_Queues[Enums::value(q)];
}

uint32_t LogicalDevice::GetQueueFamily(QueueType q) const
{
	assert(Enums::validate(q));
	return m_QueueFamilies[Enums::value(q)];
}

void LogicalDevice::DrawFrame()
{
	m_BuiltinUniformBuffers->Update();

	using namespace std::chrono_literals;
	const auto result = Get().acquireNextImageKHR(m_Swapchain->Get(), std::chrono::nanoseconds(1s).count(), *m_ImageAvailableSemaphore, nullptr);
	assert(result.result == vk::Result::eSuccess);
	const uint32_t imageIndex = result.value;

	// Submit cmd buffers
	{
		vk::SubmitInfo submitInfo;
		const vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

		const vk::Semaphore waitSemaphores[] = { m_ImageAvailableSemaphore.get() };
		submitInfo.setWaitSemaphoreCount(std::size(waitSemaphores));
		submitInfo.setPWaitSemaphores(waitSemaphores);
		submitInfo.setPWaitDstStageMask(waitStages);

		const vk::CommandBuffer cmdBuffers[] = { m_CommandBuffers[imageIndex].get() };
		submitInfo.setCommandBufferCount(std::size(cmdBuffers));
		submitInfo.setPCommandBuffers(cmdBuffers);

		const vk::Semaphore signalSempahores[] = { m_RenderFinishedSemaphore.get() };
		submitInfo.setPSignalSemaphores(signalSempahores);
		submitInfo.setSignalSemaphoreCount(std::size(signalSempahores));

		GetQueue(QueueType::Graphics).submit(submitInfo, nullptr);
	}

	// Present
	{
		vk::PresentInfoKHR presentInfo;

		const vk::Semaphore waitSemaphores[] = { m_RenderFinishedSemaphore.get() };
		presentInfo.setWaitSemaphoreCount(std::size(waitSemaphores));
		presentInfo.setPWaitSemaphores(waitSemaphores);

		const vk::SwapchainKHR swapchains[] = { m_Swapchain->Get() };
		presentInfo.setSwapchainCount(std::size(swapchains));
		presentInfo.setPSwapchains(swapchains);

		presentInfo.setPImageIndices(&imageIndex);

		vk::Result mainPresentResult = GetQueue(QueueType::Presentation).presentKHR(presentInfo);
		assert(mainPresentResult == vk::Result::eSuccess);
	}
}

void LogicalDevice::WindowResized()
{
	Log::TagMsg(TAG, "Window resized, recreating swapchain...");
	RecreateSwapchain();
}

vk::UniqueCommandBuffer LogicalDevice::AllocCommandBuffer(vk::CommandBufferLevel level) const
{
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.setLevel(level);
	allocInfo.setCommandPool(m_CommandPool.get());
	allocInfo.setCommandBufferCount(1);

	return std::move(Get().allocateCommandBuffersUnique(allocInfo).front());
}

std::vector<vk::UniqueCommandBuffer> LogicalDevice::AllocCommandBuffers(uint32_t count, vk::CommandBufferLevel level) const
{
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.setLevel(level);
	allocInfo.setCommandPool(m_CommandPool.get());
	allocInfo.setCommandBufferCount(count);

	return Get().allocateCommandBuffersUnique(allocInfo);
}

void LogicalDevice::SubmitCommandBuffers(const vk::CommandBuffer& cmdBuf, QueueType q) const
{
	SubmitCommandBuffers({ cmdBuf }, q);
}

void LogicalDevice::SubmitCommandBuffers(const std::initializer_list<vk::CommandBuffer>& cmdBufs, QueueType q) const
{
	vk::SubmitInfo submitInfo;
	submitInfo.setCommandBufferCount(cmdBufs.size());
	submitInfo.setPCommandBuffers(cmdBufs.begin());

	GetQueue(q).submit(submitInfo, nullptr);
	Get().waitIdle();
}

LogicalDevice::LogicalDevice(const std::shared_ptr<PhysicalDeviceData>& physicalDevice) :
	m_PhysicalDeviceData(physicalDevice)
{
	Log::Msg<LogType::ObjectLifetime>(__FUNCSIG__);
	ChooseQueueFamilies();

	InitDevice();
	InitDescriptorPool();
	m_BuiltinUniformBuffers.emplace(*this);

	m_ShaderGroupDataManagerInstance.emplace(*this);
	m_ShaderGroupManagerInstance.emplace(*this);
	m_MaterialDataManagerInstance.emplace(*this);
	m_MaterialManagerInstance.emplace(*this);

	InitSwapchain();
	InitRenderPass();
	InitFramebuffers();
	InitCommandPool();
	InitCommandBuffers();
	InitSemaphores();
}

LogicalDevice::~LogicalDevice()
{
	Log::Msg<LogType::ObjectLifetime>(__FUNCSIG__);

	Get().waitIdle();

	// Semaphores
	m_ImageAvailableSemaphore.reset();
	m_RenderFinishedSemaphore.reset();

	m_RenderPass.reset();
	m_Swapchain.reset();

	// Command buffers
	m_CommandBuffers.clear();

	// Command pool
	m_CommandPool.reset();

	m_TestDrawable.reset();

	m_MaterialDataManagerInstance.reset();
	m_ShaderGroupManagerInstance.reset();
	m_ShaderGroupDataManagerInstance.reset();

	// Device
	m_LogicalDevice.reset();
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
	deviceCreateInfo.setPpEnabledExtensionNames(m_PhysicalDeviceData->ChooseBestExtensionSet().data());
	deviceCreateInfo.setEnabledExtensionCount(m_PhysicalDeviceData->ChooseBestExtensionSet().size());

	m_LogicalDevice = m_PhysicalDeviceData->GetPhysicalDevice().createDeviceUnique(deviceCreateInfo);

	m_Queues[Enums::value(QueueType::Graphics)] = m_LogicalDevice->getQueue(GetQueueFamily(QueueType::Graphics), graphicsQueueIndex);
	m_Queues[Enums::value(QueueType::Presentation)] = m_LogicalDevice->getQueue(GetQueueFamily(QueueType::Presentation), presentationQueueIndex);
}

void LogicalDevice::InitDescriptorPool()
{
	vk::DescriptorPoolSize poolSize;
	poolSize.setType(vk::DescriptorType::eUniformBuffer);
	poolSize.setDescriptorCount(10);

	vk::DescriptorPoolCreateInfo createInfo;
	createInfo.setPoolSizeCount(1);
	createInfo.setPPoolSizes(&poolSize);
	createInfo.setMaxSets(10);
	createInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

	m_DescriptorPool = Get().createDescriptorPoolUnique(createInfo);
}

void LogicalDevice::InitSwapchain()
{
	Log::TagMsg(TAG, "Creating swap chain...");

	auto swapchainData = std::shared_ptr<SwapchainData>(new SwapchainData(
		m_PhysicalDeviceData->GetPhysicalDevice(), GetData().GetWindowSurface()));

	m_Swapchain.emplace(swapchainData, *this);
}

void LogicalDevice::InitRenderPass()
{
	vk::AttachmentDescription colorAttachment;
	{
		colorAttachment.setFormat(GetSwapchain().GetInitValues().m_SurfaceFormat.format);
		colorAttachment.setSamples(vk::SampleCountFlagBits::e1);

		colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
		colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);

		colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
		colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
		colorAttachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
	}

	vk::AttachmentReference colorAttachmentRef;
	{
		colorAttachmentRef.setAttachment(0);
		colorAttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	}

	vk::SubpassDescription subpass;
	{
		subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

		subpass.setColorAttachmentCount(1);
		subpass.setPColorAttachments(&colorAttachmentRef);
	}

	vk::SubpassDependency dependency;
	{
		dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
		dependency.setDstSubpass(0);

		dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		//dependency.setSrcAccessMask(0);

		dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);
	}

	vk::RenderPassCreateInfo rpCreateInfo;
	{
		rpCreateInfo.setAttachmentCount(1);
		rpCreateInfo.setPAttachments(&colorAttachment);
		rpCreateInfo.setSubpassCount(1);
		rpCreateInfo.setPSubpasses(&subpass);
		rpCreateInfo.setDependencyCount(1);
		rpCreateInfo.setPDependencies(&dependency);
	}

	m_RenderPass = Get().createRenderPassUnique(rpCreateInfo);
}

void LogicalDevice::InitFramebuffers()
{
	Log::TagMsg(TAG, "Creating framebuffers...");

	((ISwapchain_LogicalDeviceFriends*)&m_Swapchain.value())->CreateFramebuffers();
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

	m_CommandBuffers = Get().allocateCommandBuffersUnique(allocInfo);

	m_TestDrawable.emplace(*this);

	auto testTexture = Texture::Create("../statue.jpg", this);

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
			renderPassInfo.setRenderPass(m_RenderPass.get());
			renderPassInfo.setFramebuffer(framebuffer);
			renderPassInfo.renderArea.setExtent(m_Swapchain->GetInitValues().m_Extent2D);

			vk::ClearValue clearColor;
			clearColor.setColor(vk::ClearColorValue(std::array<float, 4>{ 0, 0, 0, 1 }));

			renderPassInfo.setClearValueCount(1);
			renderPassInfo.setPClearValues(&clearColor);

			cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

			m_TestDrawable->Draw(cmdBuffer.get());

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

void LogicalDevice::RecreateSwapchain()
{
	Get().waitIdle();

	((ISwapchain_LogicalDeviceFriends*)&m_Swapchain.value())->Recreate(
		std::make_shared<SwapchainData>(GetData().GetPhysicalDevice(), GetData().GetWindowSurface()));

	InitRenderPass();
	InitFramebuffers();
	InitCommandBuffers();
}

void LogicalDevice::ChooseQueueFamilies()
{
	// Try to use a single queue for everything
	const auto bestEverythingQueue = m_PhysicalDeviceData->ChooseBestQueue(true, vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer);
	if (bestEverythingQueue.has_value())
	{
		m_QueueFamilies[Enums::value(QueueType::Graphics)] =
			m_QueueFamilies[Enums::value(QueueType::Presentation)] =
			m_QueueFamilies[Enums::value(QueueType::Transfer)] =
			bestEverythingQueue->first;
	}
	else
	{
		// Separate queues
		m_QueueFamilies[Enums::value(QueueType::Graphics)] = m_PhysicalDeviceData->ChooseBestQueue(false, vk::QueueFlagBits::eGraphics)->first;
		m_QueueFamilies[Enums::value(QueueType::Transfer)] = m_PhysicalDeviceData->ChooseBestQueue(false, vk::QueueFlagBits::eTransfer)->first;
		m_QueueFamilies[Enums::value(QueueType::Presentation)] = m_PhysicalDeviceData->ChooseBestQueue(true)->first;
	}
}
