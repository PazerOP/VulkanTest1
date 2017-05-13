#pragma once
#include "Util.h"

#include <map>
#include <memory>
#include <string>

class LogicalDevice;

template<class ParentType, class ElementType> class DataStore
{
public:
	using DataStoreType = DataStore<ParentType, ElementType>;

	DataStore(LogicalDevice& device);
	virtual ~DataStore();
	static ParentType& Instance();

	virtual void Reload() = 0;

	std::shared_ptr<const ElementType> Find(const std::string& name) const;
	std::shared_ptr<ElementType> Find(const std::string& name) { return std::const_pointer_cast<ElementType>(const_this(this)->Find(name)); }

	auto begin() const { return m_Data.begin(); }
	auto begin() { return m_Data.begin(); }
	auto end() const { return m_Data.end(); }
	auto end() { return m_Data.end(); }

protected:
	LogicalDevice& m_Device;
	std::map<std::string, std::shared_ptr<ElementType>> m_Data;

private:
	bool m_Init;
	static ParentType* s_Instance;
};

template<class ParentType, class ElementType> ParentType* DataStore<ParentType, ElementType>::s_Instance;

template<class ParentType, class ElementType>
inline DataStore<ParentType, ElementType>::DataStore(LogicalDevice& device) :
	m_Device(device)
{
	m_Init = false;
	assert(!s_Instance);
	s_Instance = static_cast<ParentType*>(this);
}

template<class ParentType, class ElementType>
inline DataStore<ParentType, ElementType>::~DataStore()
{
	assert(s_Instance == static_cast<ParentType*>(this));
	s_Instance = nullptr;
}

template<class ParentType, class ElementType>
inline ParentType& DataStore<ParentType, ElementType>::Instance()
{
	if (!s_Instance)
		throw std::runtime_error(StringTools::CSFormat("Attempted to call " __FUNCSIG__ " before the {0} instance was constructed!", typeid(ParentType).name()));

	if (!s_Instance->m_Init)
	{
		s_Instance->Reload();
		s_Instance->m_Init = true;
	}

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
