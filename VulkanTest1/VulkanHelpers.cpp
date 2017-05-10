#include "stdafx.h"
#include "VulkanHelpers.h"

namespace VulkanHelpers
{
	const vk::CommandBufferBeginInfo CBBI_ONE_TIME_SUBMIT =
		vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
}