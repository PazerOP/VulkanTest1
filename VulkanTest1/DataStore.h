#pragma once
#include "Util.h"

#include <map>
#include <memory>
#include <string>

class LogicalDevice;

template<class ParentType, class ElementType, class StorageType = ElementType> class DataStore
{
public:
	using DataStoreType = DataStore<ParentType, ElementType, StorageType>;

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
	void ClearData() { m_Data.clear(); }
	void AddPair(const std::string& name, const std::shared_ptr<StorageType>& storage);
	virtual std::shared_ptr<ElementType> Transform(const std::shared_ptr<StorageType>& in) const;

	LogicalDevice& m_Device;

private:
	struct Storage
	{
		std::shared_ptr<const ElementType> Get() const;
		std::shared_ptr<ElementType> Get() { return std::const_pointer_cast<ElementType>(const_this(this)->Get()); }

	protected:
		Storage(const std::shared_ptr<StorageType>& storage) : m_Storage(storage) { }
		friend class DataStoreType;

	private:
		mutable std::shared_ptr<ElementType> m_Cached;
		std::shared_ptr<StorageType> m_Storage;
	};

	std::map<std::string, Storage> m_Data;

	bool m_Init;
	static ParentType* s_Instance;
};

template<class ParentType, class ElementType, class StorageType> ParentType* DataStore<ParentType, ElementType, StorageType>::s_Instance;

template<class ParentType, class ElementType, class StorageType>
inline DataStore<ParentType, ElementType, StorageType>::DataStore(LogicalDevice& device) :
	m_Device(device)
{
	m_Init = false;
	assert(!s_Instance);
	s_Instance = static_cast<ParentType*>(this);
}

template<class ParentType, class ElementType, class StorageType>
inline DataStore<ParentType, ElementType, StorageType>::~DataStore()
{
	assert(s_Instance == static_cast<ParentType*>(this));
	s_Instance = nullptr;
}

template<class ParentType, class ElementType, class StorageType>
inline ParentType& DataStore<ParentType, ElementType, StorageType>::Instance()
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

template<class ParentType, class ElementType, class StorageType>
inline std::shared_ptr<ElementType> DataStore<ParentType, ElementType, StorageType>::Transform(const std::shared_ptr<StorageType>& in) const
{
	assert(typeid(ElementType) == typeid(StorageType));
	return reinterpret_pointer_cast<ElementType>(in);
}

template<class ParentType, class ElementType, class StorageType>
inline std::shared_ptr<const ElementType> DataStore<ParentType, ElementType, StorageType>::Find(const std::string& name) const
{
	auto found = m_Data.find(name);
	if (found != m_Data.end())
		return found->second.Get();

	Log::Msg(__FUNCTION__ ": Unable to find a {0} named \"{1}\"", typeid(ParentType).name(), name);
	return nullptr;
}

template<class ParentType, class ElementType, class StorageType>
inline void DataStore<ParentType, ElementType, StorageType>::AddPair(const std::string& name, const std::shared_ptr<StorageType>& storage)
{
	m_Data.insert(std::make_pair(name, Storage(storage)));
}

template<class ParentType, class ElementType, class StorageType>
inline std::shared_ptr<const ElementType> DataStore<ParentType, ElementType, StorageType>::Storage::Get() const
{
	if (!m_Cached)
		m_Cached = ((DataStoreType*)&Instance())->Transform(m_Storage);

	return m_Cached;
}
