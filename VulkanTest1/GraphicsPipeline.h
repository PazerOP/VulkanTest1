#pragma once
#include "ShaderModule.h"

#include <vector>
#include <vulkan/vulkan.hpp>

class GraphicsPipeline
{
public:


private:
	std::shared_ptr<ShaderModule> m_Shaders[underlying_value(ShaderType::Count)];

	std::shared_ptr<vk::PipelineLayout> m_Layout;
};