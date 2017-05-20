#include "stdafx.h"
#include "ShaderModule.h"

#include "LogicalDevice.h"
#include "Vulkan.h"

#include <fstream>

#include <spirv_cross.hpp>

static void TestFunc(const std::vector<uint32_t>& alignedSPIRV)
{
	spirv_cross::Compiler compiler(alignedSPIRV);

	for (const auto& specConst : compiler.get_specialization_constants())
	{
		const auto& test = compiler.get_constant(specConst.id);
		const auto& testName = compiler.get_name(specConst.id);

		continue;
	}

	return;
}

ShaderModule::ShaderModule(const std::filesystem::path& path, ShaderType type, LogicalDevice& device)
{
	Log::Msg<LogType::ObjectLifetime>(__FUNCSIG__);

	m_Device = &device;
	m_Type = type;

	std::ifstream stream(path.string(), std::ios::ate | std::ios::binary);

	if (!stream.is_open())
		throw std::runtime_error(StringTools::CSFormat("Failed to open file \"{0}\"", path));

	const size_t fileSize = stream.tellg();
	std::vector<uint32_t> codeBytes((fileSize + sizeof(uint32_t) - 1) / sizeof(uint32_t));
	codeBytes.back() = 0;
	stream.seekg(0);
	stream.read((char*)codeBytes.data(), fileSize);

	vk::ShaderModuleCreateInfo createInfo;

	createInfo.setCodeSize(fileSize);
	createInfo.setPCode(codeBytes.data());

	TestFunc(codeBytes);

	m_Shader = device->createShaderModuleUnique(createInfo);
}