#include "stdafx.h"
#include "Transform.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

Transform::Transform()
{
	m_Scale = glm::vec2(1, 1);
	m_RotationRad = 0;
}

glm::mat4 Transform::ComputeMatrix() const
{
	const auto translation = glm::translate(glm::vec3(m_Translation, 1));
	const auto rotation = glm::rotate(m_RotationRad, glm::vec3(0, 1, 0));
	const auto scale = glm::scale(glm::vec3(m_Scale, 1));

	return translation * rotation * scale;
}
