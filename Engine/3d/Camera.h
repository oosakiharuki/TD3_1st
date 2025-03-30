#pragma once
#include "MyMath.h"
class Camera {
public:
	Camera();
	void Update();
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }
	void SetForY(const float& ForY) { forY = ForY; }
	void SetAspectRatio(const float aspectRatio) { aspect = aspectRatio; }
	void SetNearClip(const float Clip) { nearClip = Clip; }
	void SetFarClip(const float Clip) { farClip = Clip; }
	const Matrix4x4& GetWorldMatrix() const { return worldMatrix; }
	const Matrix4x4& GetViewMatrix() const { return viewMatrix; }
	const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix; }
	const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix; }
	const Vector3& GetRotate() const { return transform.rotate; }
	const Vector3& GetTranslate() const { return transform.translate; }
private:
	Transform transform;
	Matrix4x4 worldMatrix;
	Matrix4x4 viewMatrix;
	Matrix4x4 projectionMatrix;
	float forY;
	float aspect;
	float nearClip;
	float farClip;
	Matrix4x4 viewProjectionMatrix;
};