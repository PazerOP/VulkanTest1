#pragma once
#include "IVertexList.h"

template<class T> class VertexList final : public IVertexList
{
public:
	static std::unique_ptr<VertexList<T>> Create();

	void AddVertex(const T& vertex) { m_Vertices.push_back(vertex); }
	void AddVertex(T&& vertex) { m_Vertices.push_back(std::move(vertex)); }

	size_t GetVertexSize() const override { return sizeof(T); }
	size_t GetVertexCount() const override { return m_Vertices.size(); }
	const void* GetVertexData() const override { return m_Vertices.data(); }

	vk::VertexInputBindingDescription GetBindingDescription() const override { return T::GetBindingDescription(); }
	std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions() const override { return T::GetAttributeDescriptions(); }

private:
	VertexList() = default;

	std::vector<T> m_Vertices;
};

template<class T> using UniqueVertexList = std::unique_ptr<VertexList<T>>;

template<class T>
inline std::unique_ptr<VertexList<T>> VertexList<T>::Create()
{
	return std::unique_ptr<VertexList<T>>(new VertexList<T>());
}
