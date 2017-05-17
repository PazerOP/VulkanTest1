#include "stdafx.h"
#include "PhysicalDeviceData.h"
#include "StringTools.h"
#include "Vulkan.h"

PhysicalDeviceData::PhysicalDeviceData()
{
	m_Init = false;
}

void PhysicalDeviceData::Init(const vk::PhysicalDevice& device, const vk::SurfaceKHR& windowSurface)
{
	m_Device = device;
	m_WindowSurface = windowSurface;

	m_SupportedLayers = device.enumerateDeviceLayerProperties();
	m_Properties = device.getProperties();
	m_QueueFamilies = device.getQueueFamilyProperties();

	FindPresentationQueueFamilies();

	RateDeviceSuitability();

	m_Init = true;
}

vk::Bool32* PhysicalDeviceData::GetFeaturePtr(vk::PhysicalDeviceFeatures& features, DeviceFeature feature)
{
	return const_cast<vk::Bool32*>(GetFeaturePtr(const_cast<const vk::PhysicalDeviceFeatures&>(features), feature));
}

const vk::Bool32* PhysicalDeviceData::GetFeaturePtr(const vk::PhysicalDeviceFeatures& features, DeviceFeature feature)
{
	assert(Enums::validate(feature));

	switch (feature)
	{
	case DeviceFeature::SamplerAnisotropy:
		return &features.samplerAnisotropy;
	}

	assert(false);
	return nullptr;
}

std::shared_ptr<PhysicalDeviceData> PhysicalDeviceData::Create(const vk::PhysicalDevice& device, const vk::SurfaceKHR& windowSurface)
{
	auto retVal = std::shared_ptr<PhysicalDeviceData>(new PhysicalDeviceData());
	retVal->Init(device, windowSurface);
	return retVal;
}

PhysicalDeviceData::Suitability PhysicalDeviceData::GetSuitability() const
{
	if (m_SwapchainSuitability != SwapchainData::Suitability::Suitable)
		return Suitability::SwapChain_Unsuitable;

	return m_Suitability.value();
}

std::string PhysicalDeviceData::GetSuitabilityMessage() const
{
	if (m_SuitabilityMessageSwapchain.empty())
		return m_SuitabilityMessageBase + m_SuitabilityMessageExtra;
	else
		return m_SuitabilityMessageBase + m_SuitabilityMessageSwapchain;
}

bool PhysicalDeviceData::HasExtension(const std::string_view& name) const
{
	for (const auto& extension : GetSupportedExtensions())
	{
		if (!name.compare(extension.extensionName))
			return true;
	}

	return false;
}

bool PhysicalDeviceData::HasFeature(DeviceFeature feature) const
{
	return *GetFeaturePtr(m_Features, feature);
}

std::vector<std::pair<uint32_t, vk::QueueFamilyProperties>> PhysicalDeviceData::GetQueueFamilies(const vk::QueueFlags& queueFlags)
{
	std::vector<std::pair<uint32_t, vk::QueueFamilyProperties>> retVal;

	for (size_t i = 0; i < m_QueueFamilies.size(); i++)
	{
		const auto& current = m_QueueFamilies[i];

		if ((current.queueFlags & queueFlags) == queueFlags)
			retVal.push_back(std::make_pair((uint32_t)i, current));
	}

	return retVal;
}

std::optional<std::pair<uint32_t, vk::QueueFamilyProperties>> PhysicalDeviceData::ChooseBestQueue(bool presentation, vk::QueueFlags flags) const
{
	for (size_t i = 0; i < GetQueueFamilies().size(); i++)
	{
		const auto& current = GetQueueFamilies().at(i);
		if (current.queueFlags & flags)
			return std::make_pair((uint32_t)i, current);
	}

	return std::nullopt;
}

std::optional<std::pair<uint32_t, vk::QueueFamilyProperties>> PhysicalDeviceData::ChooseBestQueue(bool presentation) const
{
	if (!GetPresentationQueueFamilies().empty())
	{
		const auto first = GetPresentationQueueFamilies().front();
		return std::make_pair(first, GetQueueFamilies().at(first));
	}

	return std::nullopt;
}

void PhysicalDeviceData::IncludeSwapchainRating(const SwapchainData& scData)
{
	// Check swap chain support
	m_SwapchainSuitability = scData.GetSuitability();
	if (m_SwapchainSuitability != SwapchainData::Suitability::Suitable)
	{
		m_SuitabilityMessageSwapchain = StringTools::CSFormat(" unsuitable, swap chain says: {0}", scData.GetSuitabilityMessage());
		return;
	}

	m_SuitabilityMessageSwapchain.clear();
	m_SwapchainRating = scData.GetRating();
}

uint32_t PhysicalDeviceData::FindMemoryType(uint32_t typeFilter, const vk::MemoryPropertyFlags & properties) const
{
	for (uint32_t i = 0; i < m_MemoryProperties.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (m_MemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}

	throw rkrp_vulkan_exception("failed to find suitable memory type!");
}

void PhysicalDeviceData::RateDeviceSuitability()
{
	m_Rating = 0;
	m_InitData = std::make_shared<InitData>();

	m_SuitabilityMessageBase = StringTools::CSFormat("Physical physicalDevice '{0}' (id {1})", m_Properties.deviceName, m_Properties.deviceID);

	// Dedicated GPUs are usually significantly better than onboard
	if (m_Properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
		m_Rating += 10;

	// Software rendering is aids
	if (m_Properties.deviceType == vk::PhysicalDeviceType::eCpu)
		m_Rating -= 10;

	// Bonuses for onboard memory
	{
		m_MemoryProperties = m_Device.getMemoryProperties();

		for (size_t i = 0; i < m_MemoryProperties.memoryHeapCount; i++)
		{
			const auto& heap = m_MemoryProperties.memoryHeaps[i];
			const auto& heapFlags = heap.flags;

			float scalar;
			if (heapFlags & vk::MemoryHeapFlagBits::eDeviceLocal)
				scalar = 1;		// 1 point per GB of full speed, device local memory
			else
				scalar = 0.25;	// 0.25 points per GB of shared memory

			constexpr float BYTES_TO_GB = 1.0f / 1024 / 1024 / 1024;

			m_Rating += scalar * BYTES_TO_GB * heap.size;
		}
	}

	RateDeviceExtensions();
	if (m_Suitability.has_value())
		return;

	RateDeviceFeatures();
	if (m_Suitability.has_value())
		return;

	if (m_QueueFamilies.empty())
	{
		m_SuitabilityMessageExtra = StringTools::CSFormat(" unsuitable, no queue families");
		m_Suitability = Suitability::MissingQueue_All;
		return;
	}

	if (GetQueueFamilies(vk::QueueFlagBits::eGraphics).empty())
	{
		m_SuitabilityMessageExtra = StringTools::CSFormat(" unsuitable, no graphics queue");
		m_Suitability = Suitability::MissingQueue_Graphics;
		return;
	}

	if (GetPresentationQueueFamilies().empty())
	{
		m_SuitabilityMessageExtra = StringTools::CSFormat(" unsuitable, no presentation queue");
		m_Suitability = Suitability::MissingQueue_Presentation;
		return;
	}

	m_SuitabilityMessageExtra = StringTools::CSFormat(" suitable, rating {0}", m_Rating);
	m_Suitability = Suitability::Suitable;
}

void PhysicalDeviceData::RateDeviceExtensions()
{
	m_SupportedExtensions = m_Device.enumerateDeviceExtensionProperties();

	static constexpr const char* REQUIRED_EXTENSIONS[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	// A list of optional extensions to enable, along with a weight of how important they are.
	static constexpr std::pair<const char*, float> OPTIONAL_EXTENSIONS[] =
	{
		{ VK_EXT_DEBUG_MARKER_EXTENSION_NAME, 5.0f }
	};

	// Required extensions
	for (size_t i = 0; i < std::size(REQUIRED_EXTENSIONS); i++)
	{
		if (!HasExtension(REQUIRED_EXTENSIONS[i]))
		{
			m_SuitabilityMessageExtra = StringTools::CSFormat(" unsuitable, missing required extension {0}", REQUIRED_EXTENSIONS[i]);
			m_Suitability = Suitability::MissingRequiredExtension;
			return;
		}

		m_InitData->m_Extensions.push_back(REQUIRED_EXTENSIONS[i]);
	}

	// Optional extensions
	for (size_t i = 0; i < std::size(OPTIONAL_EXTENSIONS); i++)
	{
		const auto& current = OPTIONAL_EXTENSIONS[i];
		if (HasExtension(current.first))
		{
			m_Rating += current.second;
			m_InitData->m_Extensions.push_back(current.first);
		}
	}
}

void PhysicalDeviceData::RateDeviceFeatures()
{
	m_Features = m_Device.getFeatures();

	static constexpr auto REQUIRED_FEATURES = make_array<DeviceFeature>(

	);

	static constexpr std::pair<DeviceFeature, float> OPTIONAL_FEATURES[] =
	{
		{ DeviceFeature::SamplerAnisotropy, 10.0f },
	};

	for (const auto& feature : REQUIRED_FEATURES)
	{
		if (!HasFeature(feature))
		{
			m_SuitabilityMessageExtra = StringTools::CSFormat(" unsuitable, missing required feature {0}", feature);
			m_Suitability = Suitability::MissingRequiredFeature;
			return;
		}
	}

	for (const auto& feature : OPTIONAL_FEATURES)
	{
		if (HasFeature(feature.first))
		{
			m_Rating += feature.second;
			*GetFeaturePtr(m_InitData->m_Features, feature.first) = true;
		}
	}
}

void PhysicalDeviceData::FindPresentationQueueFamilies()
{
	m_PresentationQueueFamilies.clear();

	for (size_t i = 0; i < m_QueueFamilies.size(); i++)
	{
		if (m_Device.getSurfaceSupportKHR(i, m_WindowSurface))
			m_PresentationQueueFamilies.push_back(i);
	}
}