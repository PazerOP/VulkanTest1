#include "PhysicalDeviceData.h"
#include "StringTools.h"

PhysicalDeviceData::PhysicalDeviceData()
{
	m_Init = false;
}

void PhysicalDeviceData::Init(const vk::PhysicalDevice& device, const std::shared_ptr<vk::SurfaceKHR>& windowSurface)
{
	if (m_Init)
		throw programmer_error("Attempted to init PhysicalDeviceData twice!");

	m_Device = device;

	m_SupportedExtensions = device.enumerateDeviceExtensionProperties();
	m_SupportedLayers = device.enumerateDeviceLayerProperties();
	m_Properties = device.getProperties();
	m_Features = device.getFeatures();
	m_QueueFamilies = device.getQueueFamilyProperties();

	FindPresentationQueueFamilies(*windowSurface);

	RateDeviceSuitability(windowSurface);

	m_Init = true;
}

std::shared_ptr<PhysicalDeviceData> PhysicalDeviceData::Create(const vk::PhysicalDevice& device, const std::shared_ptr<vk::SurfaceKHR>& windowSurface)
{
	auto retVal = std::shared_ptr<PhysicalDeviceData>(new PhysicalDeviceData());
	retVal->Init(device, windowSurface);
	return retVal;
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

std::vector<std::pair<uint32_t, vk::QueueFamilyProperties>> PhysicalDeviceData::GetQueueFamilies(const vk::QueueFlags& queueFlags)
{
	std::vector<std::pair<uint32_t, vk::QueueFamilyProperties>> retVal;

	for (size_t i = 0; i < m_QueueFamilies.size(); i++)
	{
		const auto& current = m_QueueFamilies[i];

		if ((current.queueFlags & queueFlags) == queueFlags)
			retVal.push_back(std::make_pair(i, current));
	}

	return retVal;
}

std::optional<std::pair<uint32_t, vk::QueueFamilyProperties>> PhysicalDeviceData::ChooseBestQueue(bool presentation, vk::QueueFlags flags) const
{
	for (size_t i = 0; i < GetQueueFamilies().size(); i++)
	{
		const auto& current = GetQueueFamilies().at(i);
		if (current.queueFlags & flags)
			return std::make_pair(i, current);
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

void PhysicalDeviceData::RateDeviceSuitability(const std::shared_ptr<vk::SurfaceKHR>& windowSurface)
{
	m_Rating = 0;

	m_SuitabilityMessage = StringTools::CSFormat("Physical physicalDevice '{0}' (id {1})", m_Properties.deviceName, m_Properties.deviceID);

	// Dedicated GPUs are usually significantly better than onboard
	if (m_Properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
		m_Rating += 10;

	// Software rendering is aids
	if (m_Properties.deviceType == vk::PhysicalDeviceType::eCpu)
		m_Rating -= 10;

	// Bonuses for onboard memory
	{
		const auto memoryProperties = m_Device.getMemoryProperties();
		std::vector<std::pair<uint32_t, vk::MemoryHeapFlagBits>> memoryTypes;

		for (size_t i = 0; i < memoryProperties.memoryHeapCount; i++)
		{
			const auto& heap = memoryProperties.memoryHeaps[i];
			const auto& heapFlags = heap.flags;

			float scalar;
			if (heapFlags & vk::MemoryHeapFlagBits::eDeviceLocal)
				scalar = 1;		// 1 point per GB of full speed, device local memory
			else
				scalar = 0.25;	// 0.25 points per GB of shared memory

			static constexpr float BYTES_TO_GB = 1.0f / 1024 / 1024 / 1024;

			m_Rating += scalar * BYTES_TO_GB * heap.size;
		}
	}

	// Required extensions
	for (size_t i = 0; i < std::size(REQUIRED_EXTENSIONS); i++)
	{
		if (!HasExtension(REQUIRED_EXTENSIONS[i]))
		{
			m_SuitabilityMessage += StringTools::CSFormat(" unsuitable, missing required extension {0}", REQUIRED_EXTENSIONS[i]);
			m_Suitability = Suitability::MissingRequiredExtension;
			return;
		}

		m_BestExtensionSet.push_back(REQUIRED_EXTENSIONS[i]);
	}

	// Optional extensions
	for (size_t i = 0; i < std::size(OPTIONAL_EXTENSIONS); i++)
	{
		const auto& current = OPTIONAL_EXTENSIONS[i];
		if (HasExtension(current.first))
		{
			m_Rating += current.second;
			m_BestExtensionSet.push_back(current.first);
		}
	}

	if (m_QueueFamilies.empty())
	{
		m_SuitabilityMessage += StringTools::CSFormat(" unsuitable, no queue families");
		m_Suitability = Suitability::MissingQueue_All;
		return;
	}

	if (GetQueueFamilies(vk::QueueFlagBits::eGraphics).empty())
	{
		m_SuitabilityMessage += StringTools::CSFormat(" unsuitable, no graphics queue");
		m_Suitability = Suitability::MissingQueue_Graphics;
		return;
	}

	if (GetPresentationQueueFamilies().empty())
	{
		m_SuitabilityMessage += StringTools::CSFormat(" unsuitable, no presentation queue");
		m_Suitability = Suitability::MissingQueue_Presentation;
		return;
	}

	// Check swap chain support
	m_SwapChainData = std::make_shared<SwapchainData>(shared_from_this(), windowSurface);
	if (m_SwapChainData->GetSuitability() != SwapchainData::Suitability::Suitable)
	{
		m_SuitabilityMessage += StringTools::CSFormat(" unsuitable, swap chain says: {0}", m_SwapChainData->GetSuitabilityMessage());
		m_Suitability = Suitability::SwapChain_Unsuitable;
		return;
	}
	m_Rating += m_SwapChainData->GetRating();

	m_SuitabilityMessage += StringTools::CSFormat(" suitable, rating {0}", m_Rating);
	m_Suitability = Suitability::Suitable;
}

void PhysicalDeviceData::FindPresentationQueueFamilies(const vk::SurfaceKHR& windowSurface)
{
	m_PresentationQueueFamilies.clear();

	for (size_t i = 0; i < m_QueueFamilies.size(); i++)
	{
		if (m_Device.getSurfaceSupportKHR(i, windowSurface))
			m_PresentationQueueFamilies.push_back(i);
	}
}