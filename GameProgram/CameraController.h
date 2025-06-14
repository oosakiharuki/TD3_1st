#pragma once
#include "WorldTransform.h"
#include "MyMath.h"
#include "Camera.h"

class CameraController {
public:
	CameraController();
	~CameraController();

	// プレイヤーの位置に基づいてカメラを更新します
	void Update(Camera* camera, const Vector3& playerPosition);

	// カメラのオフセット（プレイヤーからの相対位置）を設定します
	void SetOffset(const Vector3& offset);

	// カメラのピッチ角度（見下ろす角度：度数法）を設定します
	void SetPitch(float pitchDeg);

	void SetYaw(float yawDeg);
	const float GetYaw() { return yawDeg_; }

	void SetTarget(const WorldTransform* target) { target_ = target; }

	//void SetTranslate(Vector3 translate) { cameraTranslate = translate; }

private:
	Vector3 offset_; // プレイヤーからのオフセット
	float pitchDeg_; // カメラのピッチ角度（度）
	float yawDeg_;   // カメラのヨー角度（度）

	//Vector3 cameraTranslate;
	const WorldTransform* target_ = nullptr;

	Vector3 cameraTransofrm_;
	Vector3 cameraRotate_;

	// カメラシェイク用
	float shakeIntensity_ = 0.0f;
	float shakeDuration_ = 0.0f;
	Vector3 shakeOffset_ = {0.0f, 0.0f, 0.0f};

	// カメラシェイク関数
	void UpdateShake();

public:
	// カメラシェイクを開始
	void StartShake(float intensity, float duration);
};