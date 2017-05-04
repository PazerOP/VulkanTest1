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

class LogicalDevice : public std::enable_shared_from_this<LogicalDevice>
{
public:
	LogicalDevice(const std::shared_ptr<const PhysicalDeviceData>& physicalDevice);

	const std::shared_ptr<const PhysicalDeviceData>& GetData() const { return m_PhysicalDevice; }

	const vk::Device& GetDevice() const { return m_LogicalDevice; }
	vk::Device& GetDevice() { return m_LogicalDevice; }

	const vk::Queue& GetQueue(QueueType q) const;
	vk::Queue& GetQueue(QueueType q) { return const_cast<vk::Queue&>(const_cast<const LogicalDevice*>(this)->GetQueue(q)); }

	uint32_t GetQueueFamily(QueueType q) const;

private:
	static constexpr const char TAG[] = "[LogicalDevice] ";

	void ChooseQueueFamilies();

	std::shared_ptr<const PhysicalDeviceData> m_PhysicalDevice;
	vk::Device m_LogicalDevice;

	uint32_t m_QueueFamilies[underlying_value(QueueType::Count)];
	vk::Queue m_Queues[underlying_value(QueueType::Count)];
};