#include "Door.h"
#ifdef _DEBUG
#include "ImGuiManager.h"
#include <iostream>
#endif

#include <cmath>

// Pi定数
const float PI = 3.14159265358979323846f;

// Vector3の演算子を使用するために名前空間を使用
using namespace MyMath;

Door::Door() {}

Door::~Door() { 
	delete model_;
	// サウンドリソースの解放は不要（Audioクラスが管理）
}

void Door::Init() {
	worldTransform_.Initialize();

	// "door" モデルを読み込み
	model_ = new Object3d();
	model_->Initialize();
	model_->SetModelFile("door");

	// 初期状態の設定
	openAngle_ = 0.0f;
	isAnimating_ = false;
	isDoorOpened_ = false;
	soundPlayed_ = false;

	// ドアの開く音を読み込み
	doorOpenSound_ = Audio::GetInstance()->LoadWave("sound/door_open.wav");
	
	// 初期位置を設定
	worldTransform_.translation_ = position_;
	
	// 利用する回転値をデバッグ出力
	#ifdef _DEBUG
	std::cout << "Door init with rotation: " << rotationY_ << " degrees" << std::endl;
	#endif
	
	// rotationY_の値（度数法）をラジアンに変換して直接適用
	worldTransform_.rotation_.y = rotationY_ * (PI / 180.0f);

	// 回転中心をヒンジ側に設定
	SetupPivotOffset();

	// 行列を更新
	worldTransform_.UpdateMatrix();

	// 再度回転値が正しく設定されているか確認
	#ifdef _DEBUG
	std::cout << "Door init complete, rotation set to: " << worldTransform_.rotation_.y << " radians" << std::endl;
	#endif
}

// すべてのキーが取得されているかチェックするヘルパーメソッド
bool Door::AreAllKeysObtained() const {
	// キーが設定されていない場合は開かない
	if (keys_.empty()) {
		return false;
	}

	// 取得済みのキーの数をカウント
	int obtainedCount = 0;
	for (const auto* key : keys_) {
		if (key && key->IsKeyObtained()) {
			obtainedCount++;
		}
	}

	// 指定された数以上のキーが取得されているかチェック
	// requiredKeyCount_が設定されていれば、その数を使用
	// そうでなければ、すべてのキーが必要
	int required = (requiredKeyCount_ > 0) ? requiredKeyCount_ : static_cast<int>(keys_.size());
	return obtainedCount >= required;
}

void Door::Update() {
	// プレイヤーとの衝突判定
	if (player_) {
		AABB playerAABB = player_->GetAABB();
		AABB doorAABB = GetAABB();

		if (IsCollisionAABB(playerAABB, doorAABB) && !isDoorOpened_) {
			// すべてのキーが取得されている場合、ドアに触れたフラグを立ててアニメーション開始
			if (AreAllKeysObtained() && !isDoorTouched_) {
				isDoorTouched_ = true;
				isAnimating_ = true;
			}
		}
	}

	// ドアの開閉アニメーション
	if (isAnimating_) {
		// ドアを開く（Y軸回転：約90度＝1.5ラジアン）
		if (openAngle_ < 1.5f) {
			// アニメーション開始時にサウンド再生
			if (!soundPlayed_) {
				Audio::GetInstance()->SoundPlayWave(doorOpenSound_, 1.5f, false);
				soundPlayed_ = true;
			}

			// 前回の角度を保存
			float prevAngle = openAngle_;
			// 角度を更新
			openAngle_ += 0.05f;
			// 回転角度の変化量
			float deltaAngle = openAngle_ - prevAngle;
			
			// 元の回転値を保存
			float currentRotY = worldTransform_.rotation_.y;
			
			// ドアを実際に回転させる（元の回転にアニメーションの回転を加算）
			worldTransform_.rotation_.y = currentRotY + deltaAngle;

			// 回転軸は既にSetupPivotOffsetで端に設定されているため、
			// 位置の調整は必要ない

			if (openAngle_ >= 1.5f) {
				// ドアが開き終わったらオーディオを停止
				Audio::GetInstance()->StopWave(doorOpenSound_);
				isDoorOpened_ = true;
				isAnimating_ = false;
			}
		}
	}

	// 行列更新
	worldTransform_.UpdateMatrix();
#ifdef _DEBUG
	// ImGuiによるデバッグ表示
	ImGui::Begin("Door Status");
	ImGui::Checkbox("Door Touch", &isDoorTouched_);
	ImGui::Checkbox("Door Opened", &isDoorOpened_);

	// キーの状態も表示
	int totalKeys = static_cast<int>(keys_.size());
	int obtainedKeys = 0;
	for (const auto* key : keys_) {
		if (key && key->IsKeyObtained()) {
			obtainedKeys++;
		}
	}

	ImGui::Text("Keys: %d/%d (Required: %d)", obtainedKeys, totalKeys, requiredKeyCount_);
	ImGui::End();
#endif
}

void Door::SetupPivotOffset() {
	// ドアの幅の半分（ドアモデルのローカル座標系での値）
	float doorHalfWidth = 3.0f * worldTransform_.scale_.x;
	
	// オフセット位置の計算（ドアのヒンジ側が原点になるように）
	Vector3 pivotOffset = {};
	
	// rotationY_値（度数法）に応じて回転と位置オフセットを計算
	float rotationRadians = rotationY_ * (PI / 180.0f);
	
	// X方向のオフセット（回転に応じて計算）
	pivotOffset.x = -doorHalfWidth * cosf(rotationRadians);
	// Z方向のオフセット
	pivotOffset.z = -doorHalfWidth * sinf(rotationRadians);
	
	// 位置を調整（元の位置 + オフセット）
	worldTransform_.translation_.x = position_.x + pivotOffset.x;
	worldTransform_.translation_.z = position_.z + pivotOffset.z;
}

void Door::Draw() { model_->Draw(worldTransform_); }

AABB Door::GetAABB(){
	// ドアのローカル座標：
	//   min = (-3, 0, -1), max = (3, 6, 1)
	//   中心 = (0, 3, 0)
	//   半サイズ(=halfExtents) = (3, 3, 1)
	Vector3 localCenter = {0.0f, 3.0f, 0.0f};
	Vector3 halfExtents = { 3.0f, 3.0f, 1.0f };

	// 回転角度に応じて調整
	// 90度および270度回転の場合はX,Zの半径を入れ替え
	float normalAngle = rotationY_;
	// 角度を0ー359の範囲に正規化
	while (normalAngle < 0) normalAngle += 360.0f;
	while (normalAngle >= 360.0f) normalAngle -= 360.0f;
	
	if ((normalAngle > 45 && normalAngle < 135) || (normalAngle > 225 && normalAngle < 315)) {
		// ほぼ90度または270度の場合、XとZを入れ替え
		halfExtents = { 1.0f, 3.0f, 3.0f };
	}
	
	// スケールを要素ごとに乗算
	Vector3 scaledCenter = {localCenter.x * worldTransform_.scale_.x, localCenter.y * worldTransform_.scale_.y, localCenter.z * worldTransform_.scale_.z};

	Vector3 scaledHalfExtents = {halfExtents.x * worldTransform_.scale_.x, halfExtents.y * worldTransform_.scale_.y, halfExtents.z * worldTransform_.scale_.z};

	// ドアのワールド中心 = ピボット位置(translation_) + scaledCenter
	Vector3 doorCenter = {worldTransform_.translation_.x + scaledCenter.x, worldTransform_.translation_.y + scaledCenter.y, worldTransform_.translation_.z + scaledCenter.z};

	// AABBを計算
	AABB doorAABB;
	doorAABB.min = {doorCenter.x - scaledHalfExtents.x, doorCenter.y - scaledHalfExtents.y, doorCenter.z - scaledHalfExtents.z};
	doorAABB.max = {doorCenter.x + scaledHalfExtents.x, doorCenter.y + scaledHalfExtents.y, doorCenter.z + scaledHalfExtents.z};
	return doorAABB;
}