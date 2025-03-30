#include "Camera.h"
#include "WinApp.h"
using namespace MyMath;
Camera::Camera()
	: transform({ { 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,-50.0f } })
	, forY(0.45f)
	, aspect(float(WinApp::kClientWidth) / float(WinApp::kClientHeight))
	, nearClip(0.1f)
	, farClip(100.0f)
	, worldMatrix(MakeAffineMatrix(transform.scale, transform.rotate, transform.translate))
	, viewMatrix(Inverse(worldMatrix))
	, projectionMatrix(MakePerspectiveFovMatrix(forY, aspect, nearClip, farClip))
	, viewProjectionMatrix(Multiply(viewMatrix, projectionMatrix))
{}
void Camera::Update() {
	worldMatrix = MakeAffineMatrix(transform.scale,transform.rotate,transform.translate);
	viewMatrix = Inverse(worldMatrix);
	projectionMatrix = MakePerspectiveFovMatrix(forY,aspect,nearClip,farClip);
	viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
}