#pragma once

class Transform
{
public:
	Transform();

	const glm::vec2& GetTranslation() const { return m_Translation; }
	void SetTranslation(const glm::vec2& translation) { m_Translation = translation; }
	void SetTranslation(float x, float y) { m_Translation = glm::vec2(x, y); }

	const glm::vec2& GetScale() const { return m_Scale; }
	void SetScale(const glm::vec2& scale) { m_Scale = scale; }

	float GetRotationRad() const { return m_RotationRad; }
	void SetRotationRad(float radians) { m_RotationRad = radians; }
	float GetRotationDeg() const { return glm::degrees(m_RotationRad); }
	void SetRotationDeg(float degrees) { m_RotationRad = glm::radians(degrees); }

	glm::mat4 ComputeMatrix() const;

private:
	glm::vec2 m_Translation;
	glm::vec2 m_Scale;
	float m_RotationRad;
};