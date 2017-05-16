#include "stdafx.h"
#include "Transform.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

Transform::Transform(const glm::vec2& translation, const glm::vec2& scale, float rotationRad) :
	m_Translation(translation),
	m_Scale(scale),
	m_RotationRad(rotationRad)
{
}

glm::mat4 Transform::ComputeMatrix() const
{
	glm::mat4 retVal;
	retVal = glm::translate(glm::vec3(m_Translation, 0));
	retVal = glm::rotate(retVal, m_RotationRad, glm::vec3(0, 0, 1));
	
	retVal = glm::scale(retVal, glm::vec3(m_Scale, 1));

	return retVal;
}
