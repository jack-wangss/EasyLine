#include "Camera.h"

namespace EasyLine {

Camera::Camera(float width, float height)
	: m_Width(width), m_Height(height), m_AspectRatio(width / height)
{
	m_ProjectionMatrix = glm::ortho(-m_AspectRatio * m_Zoom, m_AspectRatio * m_Zoom, -m_Zoom, m_Zoom, -1.0f, 1.0f);
	m_ViewMatrix = glm::mat4(1.0f);
	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void Camera::OnResize(float width, float height)
{
	m_Width = width;
	m_Height = height;
	m_AspectRatio = width / height;
	RecalculateViewMatrix();
}

void Camera::RecalculateViewMatrix()
{
	m_ProjectionMatrix = glm::ortho(-m_AspectRatio * m_Zoom, m_AspectRatio * m_Zoom, -m_Zoom, m_Zoom, -1.0f, 1.0f);
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), { m_Position.x, m_Position.y, 0.0f });
	m_ViewMatrix = glm::inverse(transform);
	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

}
