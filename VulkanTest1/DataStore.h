#pragma once
#include "Util.h"

#include <map>
#include <memory>
#include <string>

class LogicalDevice;

template<class ParentType, class ElementType> class DataStore
{
public:
	DataStore(LogicalDevice& device);
	virtual ~DataStore();
	static ParentType& Instance();

	virtual void Reload() = 0;

	std::shared_ptr<const ElementType> Find(const std::string& name) const;
	std::shared_ptr<ElementType> Find(const std::string& name) { return std::const_pointer_cast<ElementType>(const_this(this)->Find(name)); }

protected:
	static ParentType* s_Instance;

	LogicalDevice& m_Device;
	std::map<std::string, std::shared_ptr<ElementType>> m_Data;
};

template<class ParentType, class ElementType>
inline DataStore<ParentType, ElementType>::DataStore(LogicalDevice& device) :
	m_Device(device)
{
	Reload();

	assert(!s_Instance);
	s_Instance = this;
}

template<class ParentType, class ElementType>
inline DataStore<ParentType, ElementType>::~DataStore()
{
	assert(s_Instance == this);
	s_Instance = nullptr;
}

template<class ParentType, class ElementType>
inline ParentType& DataStore<ParentType, ElementType>::Instance()
{
	if (!s_Instance)
		throw std::runtime_error(StringTools::CSFormat("Attempted to call " __FUNCSIG__ " before the {0} instance was constructed!", typeid(ParentType).name()));

	return *s_Instance;
}

template<class ParentType, class ElementType>
inline std::shared_ptr<const ElementType> DataStore<ParentType, ElementType>::Find(const std::string& name) const
{
	auto found = m_Data.find(name);
	if (found != m_Data.end())
		return found->second;

	Log::Msg(__FUNCTION__ ": Unable to find a {0} named \"{1}\"", typeid(ParentType).name(), name);
	return nullptr;
}
