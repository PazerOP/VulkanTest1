#include "stdafx.h"
#include "TestDrawable.h"

#include "MaterialManager.h"

#include "SimpleVertex.h"
#include "VertexList.h"

#include <random>

static UniqueVertexList<SimpleVertex> GetTestVertexList()
{
	UniqueVertexList<SimpleVertex> retVal = VertexList<SimpleVertex>::Create();

	SimpleVertex v[4] =
	{
		{ { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
		{ { 0.5f, -0.5f }, { 0.5f, 1.0f, 0.0f } },
		{ { 0.5f, 0.5f }, { 0.0f, 1.0f, 1.0f } },
		{ { -0.5f, 0.5f }, { 0.5f, 0.0f, 1.0f } },
	};

	retVal->AddVertex({ v[0], v[1], v[2], v[2], v[3], v[0] });

	return retVal;
}

TestDrawable::TestDrawable(LogicalDevice& device) :
	Drawable(device)
{
	m_Transform.SetTranslation(glm::vec2(300, 200));
	m_Transform.SetScale(glm::vec2(300));

	m_Material = MaterialManager::Instance().Find("test_material");
	m_Mesh = Mesh::Create(GetTestVertexList());
}

void TestDrawable::Update()
{
	Drawable::Update();
	//std::uniform
}
