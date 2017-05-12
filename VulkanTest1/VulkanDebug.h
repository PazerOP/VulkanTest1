#pragma once

#include <map>

class VulkanDebug
{
public:
	VulkanDebug() = delete;

	static const std::string* GetObjectName(uint64_t objectID);
	static void SetObjectName(const vk::Image& image, const std::string& name);
	static void SetObjectName(uint64_t objectID, const std::string& name);

private:
	static std::map<uint64_t, std::string> s_DebugNames;
};