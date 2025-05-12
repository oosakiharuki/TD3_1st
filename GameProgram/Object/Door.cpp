#include "Door.h"
#ifdef _DEBUG
#include "ImGuiManager.h"
#endif

#include <numbers>

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
	
	// normal_の値に応じて初期回転を設定
	if (normal_ == 90) {
		worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2;
	} else if (normal_ == 180) {
		worldTransform_.rotation_.y = std::numbers::pi_v<float>;
	} else if (normal_ == 270) {
		worldTransform_.rotation_.y = -std::numbers::pi_v<float> / 2;
	}

	// 行列を更新
	worldTransform_.UpdateMatrix();
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

			openAngle_ += 0.05f;
			worldTransform_.rotation_.y = openAngle_;

			// ドアのヒンジ部分を軸に回転させるための処理
			// ドアの幅の半分の値（X方向のオフセット）
			float doorHalfWidth = 3.0f * worldTransform_.scale_.x;
			
			// 回転に応じた位置補正（回転前の位置に戻す）
			Vector3 offsetPosition = {};
			
			// normal_の値に応じてオフセット方向を調整
			if (normal_ == 0) {
				// Z軸方向を向いているドア（デフォルト）
				// X軸方向にオフセット（ドアの右端が回転軸）
				offsetPosition.x = doorHalfWidth * (1.0f - cosf(openAngle_));
				offsetPosition.z = doorHalfWidth * sinf(openAngle_);
			} else if (normal_ == 90) {
				// X軸方向を向いているドア（90度回転）
				// Z軸方向にオフセット
				offsetPosition.z = -doorHalfWidth * (1.0f - cosf(openAngle_));
				offsetPosition.x = doorHalfWidth * sinf(openAngle_);
			} else if (normal_ == 180) {
				// Z軸の逆方向を向いているドア（180度回転）
				offsetPosition.x = -doorHalfWidth * (1.0f - cosf(openAngle_));
				offsetPosition.z = -doorHalfWidth * sinf(openAngle_);
			} else if (normal_ == 270) {
				// X軸の逆方向を向いているドア（270度回転）
				offsetPosition.z = doorHalfWidth * (1.0f - cosf(openAngle_));
				offsetPosition.x = -doorHalfWidth * sinf(openAngle_);
			}
			
			// 位置を更新
			// Vector3の加算を明示的に行う
			worldTransform_.translation_.x = position_.x + offsetPosition.x;
			worldTransform_.translation_.y = position_.y + offsetPosition.y;
			worldTransform_.translation_.z = position_.z + offsetPosition.z;

			if (openAngle_ >= 1.5f) {
				isDoorOpened_ = true;
				isAnimating_ = false;
				worldTransform_.rotation_.y = openAngle_;
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

void Door::Draw() { model_->Draw(worldTransform_); }

AABB Door::GetAABB(){
	// ドアのローカル座標：
	//   min = (-3, 0, -1), max = (3, 6, 1)
	//   中心 = (0, 3, 0)
	//   半サイズ(=halfExtents) = (3, 3, 1)
	Vector3 localCenter = {0.0f, 3.0f, 0.0f};
	Vector3 halfExtents = { 3.0f, 3.0f, 1.0f };

	if (!isDoorOpened_) {
		if (normal_ == 90) {
			halfExtents = { 1.0f, 3.0f, 3.0f };
			worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2;
		}
		if (normal_ == 270) {
			halfExtents = { 1.0f, 3.0f, 3.0f };
			worldTransform_.rotation_.y = - std::numbers::pi_v<float> / 2;
		}
		if (normal_ == 180) {
			worldTransform_.rotation_.y = std::numbers::pi_v<float>;
		}
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