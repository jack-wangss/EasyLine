#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace EasyLine {

class Camera
{
public:
	Camera(float width, float height);

	void OnResize(float width, float height);

	const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

	void SetPosition(const glm::vec2& position) { m_Position = position; RecalculateViewMatrix(); }
	const glm::vec2& GetPosition() const { return m_Position; }

	void SetZoom(float zoom) { m_Zoom = zoom; RecalculateViewMatrix(); }
	float GetZoom() const { return m_Zoom; }

private:
	void RecalculateViewMatrix();

private:
	glm::mat4 m_ProjectionMatrix;
	glm::mat4 m_ViewMatrix;
	glm::mat4 m_ViewProjectionMatrix;

	glm::vec2 m_Position = { 0.0f, 0.0f };
	float m_Zoom = 1.0f;

	float m_AspectRatio = 0.0f;
	float m_Width = 0.0f, m_Height = 0.0f;
};

}
