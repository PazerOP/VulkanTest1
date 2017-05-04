#pragma once
#include "PhysicalDeviceData.h"
#include "Util.h"
#include "Vulkan.h"

#include <memory>

class LogicalDevice
{
public:
	LogicalDevice(const std::shared_ptr<PhysicalDeviceData>& physicalDevice);


private:
	std::shared_ptr<PhysicalDeviceData> m_PhysicalDevice;

public:
};