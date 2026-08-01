#pragma once

#include "cyMatrix.h"
#include "cyVector.h"


class Camera
{
public:
	Camera(
		cy::Vec3f target = cy::Vec3f(0, 0, 0),
		float distance = 50.0f
	);

	~Camera() = default;

	// Input
	void ProcessMouseOrbit(float deltaX, float deltaY);
	void ProcessMouseZoom(float deltaY);


	// Settings
	void SetAspectRatio(float aspect);
	void ToggleProjectionMode();
	void SetTarget(const cy::Vec3f& target);
	void SetDistance(float distance);


	// Output
	cy::Matrix4f GetViewMatrix() const;
	cy::Matrix4f GetProjectionMatrix() const;

	cy::Vec3f GetPosition() const;
	bool IsPerspective() const;


private:

	cy::Vec3f m_Target;

	float m_Distance;

	float m_Pitch = 0.0f;
	float m_Yaw = 0.0f;

	bool m_IsPerspective = true;

	float m_AspectRatio = 16.0f / 9.0f;

	float m_FovY = 45.0f * 3.14159f / 180.0f;

	float m_NearPlane = 0.1f;

	float m_FarPlane = 1000.0f;

	cy::Matrix4f OrthoMatrix(
		float left,
		float right,
		float bottom,
		float top,
		float nearVal,
		float farVal
	) const;
};
