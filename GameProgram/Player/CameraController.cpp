#include "CameraController.h"
#include "ImGuiManager.h"
#include "Player.h"

CameraController::CameraController() : offset_{0.0f, 5.0f, -20.0f}, pitchDeg_(10.0f) {}

CameraController::~CameraController() {}

void CameraController::SetOffset(const Vector3& offset) { offset_ = offset; }

void CameraController::SetPitch(float pitchDeg) { pitchDeg_ = pitchDeg; }

void CameraController::SetYaw(float yawDeg) { yawDeg_ = yawDeg; }

void CameraController::Update(Camera* camera, const Vector3& playerPosition) {
	if (camera == nullptr) {
		return;
	}

	// 回転角をラジアンに変換
	float pitchRad = pitchDeg_ * (3.14159265f / 180.0f);
	float yawRad = yawDeg_ * (3.14159265f / 180.0f);

	// プレイヤーからの基本距離（cameraTranslate.zで調整可能）
	float distance = 35.0f;

	// ピッチ角に基づく水平距離を計算
	float horizontalDistance = distance * std::cos(pitchRad);

	// ピッチ角に基づく垂直オフセットを計算
	float verticalOffset = distance * std::sin(pitchRad);

	// 目標位置を計算
	Vector3 targetPosition;
	targetPosition.x = playerPosition.x - horizontalDistance * std::sin(yawRad);
	targetPosition.y = playerPosition.y + 5.0f + verticalOffset;
	targetPosition.z = playerPosition.z - horizontalDistance * std::cos(yawRad);

	// スムージング係数（値が大きいほど即座に追従）
	const float smoothFactor = 0.7f;

	// 現在位置から目標位置へ徐々に移動（線形補間）
	cameraTransofrm_.x += (targetPosition.x - cameraTransofrm_.x) * smoothFactor;
	cameraTransofrm_.y += (targetPosition.y - cameraTransofrm_.y) * smoothFactor;
	cameraTransofrm_.z += (targetPosition.z - cameraTransofrm_.z) * smoothFactor;

	// プレイヤーを見るようにカメラの回転を徐々に更新
	// 回転のスムージング係数（値が大きいほど即座に追従）
	const float rotationSmoothFactor = 0.7f;

	// 現在の回転から目標回転へ徐々に移動
	cameraRotate_.x += (pitchRad - cameraRotate_.x) * rotationSmoothFactor;
	cameraRotate_.y += (yawRad - cameraRotate_.y) * rotationSmoothFactor;
	cameraRotate_.z = 0.0f;

#ifdef _DEBUG
	ImGui::Begin("camera");
	ImGui::DragFloat3("cameraTranslate",  &cameraTransofrm_.x);
	ImGui::DragFloat3("cameraRotate", &cameraRotate_.x);
	ImGui::DragFloat3("offset", &offset_.x);
	ImGui::End();
#endif

	// カメラ行列の更新と転送

	camera->SetRotate(cameraRotate_);
	camera->SetTranslate(cameraTransofrm_);

	//camera->UpdateViewMatrix();
	//camera->TransferMatrix();
}