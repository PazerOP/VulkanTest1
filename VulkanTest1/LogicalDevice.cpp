#include "stdafx.h"
#include "LogicalDevice.h"

#include "Log.h"

LogicalDevice::LogicalDevice(const std::shared_ptr<const PhysicalDeviceData>& physicalDevice)
{
	m_PhysicalDevice = physicalDevice;

	ChooseQueueFamilies();

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
