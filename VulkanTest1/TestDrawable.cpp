#include "stdafx.h"
#include "TestDrawable.h"

#include "MaterialManager.h"

#include "SimpleVertex.h"
#include "VertexList.h"

#include <random>

static UniqueVertexList<SimpleVertex> GetTestVertexList()
{
	UniqueVertexList<SimpleVertex> retVal = VertexList<SimpleVertex>::Create();

	constexpr float aspect = 12.0f / 8.0f;
	constexpr SimpleVertex v[4] =
	{
		SimpleVertex(glm::vec2(-0.5 * aspect, -0.5), glm::vec3(1, 0, 0), glm::vec2(0, 0)),
		SimpleVertex(glm::vec2(0.5 * aspect, -0.5), glm::vec3(0.5, 1, 0), glm::vec2(1, 0)),
		SimpleVertex(glm::vec2(0.5 * aspect, 0.5), glm::vec3(0, 1, 1), glm::vec2(1, 1)),
		SimpleVertex(glm::vec2(-0.5 * aspect, 0.5), glm::vec3(0.5, 0, 1), glm::vec2(0, 1)),
	};

	retVal->AddVertex({ v[0], v[1], v[2], v[2], v[3], v[0] });

	return retVal;
}

TestDrawable::TestDrawable(LogicalDevice& device) :
	Drawable(device)
{
	m_Transform.SetTranslation(glm::vec2(300, 0));
	m_Transform.SetScale(glm::vec2(300));

	m_Material = MaterialManager::Instance().Find("test_material");
	m_Mesh = Mesh::Create(GetTestVertexList());
}

void TestDrawable::Update()
{
	Drawable::Update();
	//std::uniform
}
