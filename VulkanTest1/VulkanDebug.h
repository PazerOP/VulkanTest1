#pragma once

class VulkanDebug
{
public:
	VulkanDebug() = delete;

	static void SetObjectName(vk::Device device, const std::string& name);
};