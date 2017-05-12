#pragma once
#include <map>
#include <memory>

class LogicalDevice;
class ShaderGroup;

class ShaderGroupManager
{
public:
	ShaderGroupManager(LogicalDevice& device);
	~ShaderGroupManager();
	static ShaderGroupManager& Instance();

	void Reload();

	std::shared_ptr<const ShaderGroup> FindShaderGroup(const std::string& name) const;
	std::shared_ptr<ShaderGroup> FindShaderGroup(const std::string& name) { return std::const_pointer_cast<ShaderGroup>(const_this(this)->FindShaderGroup(name)); }

private:
	static ShaderGroupManager* s_Instance;

	LogicalDevice& m_Device;

	std::map<std::string, std::shared_ptr<ShaderGroup>> m_ShaderGroups;
};