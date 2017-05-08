#pragma once
#include "IVertexList.h"

template<class T> class VertexList final : public IVertexList
{
public:
	static std::unique_ptr<VertexList<T>> Create();

	void AddVertex(const T& vertex);
	void AddVertex(std::initializer_list<T> vertices);

	size_t GetVertexSize() const override { return sizeof(T); }
	size_t GetVertexCount() const override { return m_Vertices.size(); }
	const void* GetVertexData() const override { return m_Vertices.data(); }

	size_t GetIndexSize() const override { return sizeof(uint32_t); }
	size_t GetIndexCount() const override { return m_Indices.size(); }
	const void* GetIndexData() const override { return m_Indices.data(); }

	vk::VertexInputBindingDescription GetBindingDescription() const override { return T::GetBindingDescription(); }
	std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions() const override { return T::GetAttributeDescriptions(); }

private:
	VertexList() = default;

	std::vector<T> m_Vertices;
	std::vector<uint32_t> m_Indices;
};

template<class T> using UniqueVertexList = std::unique_ptr<VertexList<T>>;

template<class T>
inline std::unique_ptr<VertexList<T>> VertexList<T>::Create()
{
	return std::unique_ptr<VertexList<T>>(new VertexList<T>());
}

template<class T>
inline void VertexList<T>::AddVertex(const T& vertex)
{
	auto found = std::find(m_Vertices.begin(), m_Vertices.end(), vertex);
	if (found != m_Vertices.end())
	{
		assert((found - m_Vertices.begin()) <= std::numeric_limits<uint32_t>::max());
		m_Indices.push_back(uint32_t(found - m_Vertices.begin()));
	}
	else
	{
		m_Indices.push_back(m_Vertices.size());
		m_Vertices.push_back(vertex);
	}
}

template<class T>
inline void VertexList<T>::AddVertex(std::initializer_list<T> vertices)
{
	for (const T& v : vertices)
		AddVertex(std::move(v));
}
