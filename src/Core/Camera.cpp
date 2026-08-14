// SPDX-License-Identifier: MIT
/// @file Camera.cpp
/// @brief 摄像机类的实现文件
/// @details 该文件实现了 Camera 类的核心功能，包括视图矩阵和投影矩阵的计算，以及鼠标交互处理。
/// @author MaiX
/// @date 2026-08-02


#include "Camera.h"

#include <cmath>

Camera::Camera(
	cy::Vec3f target,
	float distance)
	: m_Target(target),
	m_Distance(distance)
{

}

void Camera::ProcessMouseOrbit(
	float deltaX,
	float deltaY
)
{
	m_Yaw += deltaX * 0.01f;
	m_Pitch += deltaY * 0.01f;
}


void Camera::ProcessMouseZoom(
	float deltaY
)
{
	m_Distance += deltaY * 0.1f;

	if (m_Distance < 0.1f) m_Distance = 0.1f;
}


void Camera::SetAspectRatio(
	float aspect
)
{
	m_AspectRatio = aspect;
}


void Camera::ToggleProjectionMode()
{
	m_IsPerspective = !m_IsPerspective;
}


void Camera::SetTarget(
	const cy::Vec3f& target
)
{
	m_Target = target;
}


void Camera::SetDistance(
	float distance
)
{
	m_Distance = distance;
}

void Camera::SetClipPlanes(float nearPlane, float farPlane)
{
	if (nearPlane <= 0.0f || farPlane <= nearPlane)
		return;

	m_NearPlane = nearPlane;
	m_FarPlane = farPlane;
}


cy::Matrix4f Camera::GetViewMatrix() const
{
	cy::Matrix4f rotation =
		cy::Matrix4f::RotationX(m_Pitch) *
		cy::Matrix4f::RotationY(m_Yaw);

	cy::Matrix4f translation =
		cy::Matrix4f::Translation(
			cy::Vec3f(0, 0, -m_Distance)
		);

	return translation * rotation;
}


cy::Matrix4f Camera::GetProjectionMatrix() const
{
	cy::Matrix4f proj;

	if (m_IsPerspective)
	{
		proj.SetPerspective(
			m_FovY,
			m_AspectRatio,
			m_NearPlane,
			m_FarPlane
		);
	}
	else
	{
		float halfHeight = m_Distance * tan(m_FovY / 2.0f);
		float halfWidth = halfHeight * m_AspectRatio;
		proj = OrthoMatrix(
			-halfWidth, halfWidth,
			-halfHeight, halfHeight,
			m_NearPlane, m_FarPlane
		);
	}

	return proj;
}


cy::Vec3f Camera::GetPosition() const
{
	return cy::Vec3f(
		0,
		0,
		m_Distance
	);
}


bool Camera::IsPerspective() const
{
	return m_IsPerspective;
}


cy::Matrix4f Camera::OrthoMatrix(
	float left, float right,
	float bottom, float top,
	float nearVal, float farVal
) const
{
	cy::Matrix4f m;

	m.Zero();

	m.cell[0] = 2.0f / (right - left);
	m.cell[5] = 2.0f / (top - bottom);
	m.cell[10] = -2.0f / (farVal - nearVal);
	m.cell[12] = -(right + left) / (right - left);
	m.cell[13] = -(top + bottom) / (top - bottom);
	m.cell[14] = -(farVal + nearVal) / (farVal - nearVal);
	m.cell[15] = 1.0f;

	return m;
}
