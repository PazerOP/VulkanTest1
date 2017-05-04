#include "LogicalDevice.h"

#include "Log.h"

LogicalDevice::LogicalDevice(const std::shared_ptr<PhysicalDeviceData>& physicalDevice)
{
	Log::TagMsg(TAG, "Creating logical device...");

	const uint32_t graphicsQueueFamily = physicalDevice->GetQueueFamilies(vk::QueueFlagBits::eGraphics).front().first;

	// Check if our presentation queue is the same as our graphics queue
	const uint32_t presentationQueueFamily = physicalDevice->GetPresentationQueueFamilies().front();

	std::vector<vk::DeviceQueueCreateInfo> dqCreateInfos;

	const float queuePriority = 1;

	// Graphics queue
	uint32_t graphicsQueueIndex;
	{
		vk::DeviceQueueCreateInfo graphicsQueue;
		graphicsQueue.setQueueCount(1);
		graphicsQueue.setQueueFamilyIndex(graphicsQueueFamily);
		graphicsQueue.setPQueuePriorities(&queuePriority);
		dqCreateInfos.push_back(graphicsQueue);
		graphicsQueueIndex = dqCreateInfos.size() - 1;
	}

	// Presentation queue, might be the same as the graphics queue
	uint32_t presentationQueueIndex = graphicsQueueIndex;
	if (presentationQueueFamily != graphicsQueueFamily)
	{
		vk::DeviceQueueCreateInfo presentationQueue;
		presentationQueue.setQueueCount(1);
		presentationQueue.setQueueFamilyIndex(presentationQueueFamily);
		presentationQueue.setPQueuePriorities(&queuePriority);
		dqCreateInfos.push_back(presentationQueue);
		presentationQueueIndex = dqCreateInfos.size() - 1;
	}

	vk::PhysicalDeviceFeatures features;

	vk::DeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.setQueueCreateInfoCount(dqCreateInfos.size());
	deviceCreateInfo.setPQueueCreateInfos(dqCreateInfos.data());

	// Extensions
	static constexpr const char* DEVICE_EXTENSIONS[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};
	deviceCreateInfo.setPpEnabledExtensionNames(DEVICE_EXTENSIONS);
	deviceCreateInfo.setEnabledExtensionCount(sizeof(DEVICE_EXTENSIONS) / sizeof(DEVICE_EXTENSIONS[0]));

	m_LogicalDevice = m_PhysicalDeviceData->GetPhysicalDevice().createDevice(deviceCreateInfo);

	m_Queues[underlying_value(QueueType::Graphics)] = m_LogicalDevice.getQueue(graphicsQueueFamily, graphicsQueueIndex);

	// Potentially shared
	if (presentationQueueIndex == graphicsQueueIndex)
		m_Queues[underlying_value(QueueType::Presentation)] = m_Queues[underlying_value(QueueType::Graphics)];
	else
		m_Queues[underlying_value(QueueType::Presentation)] = m_LogicalDevice.getQueue(presentationQueueFamily, presentationQueueIndex);
}