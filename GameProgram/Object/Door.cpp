#include "Door.h"
#ifdef _DEBUG
#include "ImGuiManager.h"
#endif

// 円周率の定義
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <tuple>

// 静的変数の定義
std::map<int, std::tuple<float, float, float>> Door::doorRotations_;

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
	isAnimating_ = false;
	isDoorOpened_ = false;
	soundPlayed_ = false;

	// ドアの開く音を読み込み
	doorOpenSound_ = Audio::GetInstance()->LoadWave("sound/door_open.wav");

	// 初期回転角度をnormal_値に基づいてラジアンに変換
	float normalRad = static_cast<float>(M_PI) / 2.0f;
	
	if (normal_ == 90) {
		normalRad = static_cast<float>(M_PI) / 2.0f; // 90度
	}
	else if (normal_ == 180) {
		normalRad = static_cast<float>(M_PI);     // 180度
	}
	else if (normal_ == 270) {
		normalRad = -static_cast<float>(M_PI) / 2.0f; // 270度 (-90度)
	}
	
	// 初期角度の設定
	worldTransform_.rotation_.y = normalRad;
	openAngle_ = normalRad; // 現在の回転角度を記録

	// デバッグ表示用に度数法にも変換して記録
	initialRotation_ = normal_; // normal_は度数法ですでに保存されている

	// 初期位置を設定
	worldTransform_.translation_ = position_;

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

bool Door::LoadRotationsFromCSV(const std::string& filePath) {
    // 以前のデータをクリア
    doorRotations_.clear();

    // CSVファイルを開く
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filePath << std::endl;
        return false;
    }

    std::string line;
    // コメント行をスキップ
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        std::string token;
        int doorID;
        float initialRotation, targetRotation, rotationSpeed;

        // doorIDを読み込む
        if (std::getline(iss, token, ',')) {
            doorID = std::stoi(token);
        } else {
            continue;
        }

        // initialRotationを読み込む
        if (std::getline(iss, token, ',')) {
            initialRotation = std::stof(token);
        } else {
            continue;
        }

        // targetRotationを読み込む
        if (std::getline(iss, token, ',')) {
            targetRotation = std::stof(token);
        } else {
            continue;
        }

        // rotationSpeedを読み込む
        if (std::getline(iss, token, ',')) {
            rotationSpeed = std::stof(token);
        } else {
            continue;
        }

        // マップに保存
        doorRotations_[doorID] = std::make_tuple(initialRotation, targetRotation, rotationSpeed);
    }

    file.close();
    return !doorRotations_.empty();
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
		// カスタム回転を使用する場合
		if (useCustomRotation_) {
			// 度数法からラジアンに変換
			// normal_値に合わせて目標角度を計算
			float currentRotationRad = worldTransform_.rotation_.y; // 現在の角度（ラジアン）
			float targetRotationRad = openAngle_ + (targetRotation_ * (static_cast<float>(M_PI) / 180.0f)); // 目標角度
			
			// アニメーション終了条件の確認
            if (fabsf(worldTransform_.rotation_.y - targetRotationRad) < 0.01f) {
                // 目標角度に達したらアニメーション終了
                // ドアが開き終わったらオーディオを停止
                Audio::GetInstance()->StopWave(doorOpenSound_);
                isDoorOpened_ = true;
                isAnimating_ = false;
                // 正確な目標角度を設定
                worldTransform_.rotation_.y = targetRotationRad;
                // openAngle_も更新
                openAngle_ = targetRotationRad;
                return; // アニメーション終了したら早期リターン
            }
			
			// 現在の角度が目標角度に達していない場合
			// アニメーション開始時にサウンド再生
			if (!soundPlayed_) {
				Audio::GetInstance()->SoundPlayWave(doorOpenSound_, 1.5f, false);
				soundPlayed_ = true;
			}

			// 回転速度をラジアンに変換
			float speedRad = rotationSpeed_ * (static_cast<float>(M_PI) / 180.0f);
			
			// 角度を更新（目標角度に近づける）
			if (targetRotationRad > worldTransform_.rotation_.y) {
				worldTransform_.rotation_.y += speedRad;
				if (worldTransform_.rotation_.y > targetRotationRad) {
					worldTransform_.rotation_.y = targetRotationRad;
				}
			} else {
				worldTransform_.rotation_.y -= speedRad;
				if (worldTransform_.rotation_.y < targetRotationRad) {
					worldTransform_.rotation_.y = targetRotationRad;
				}
			}

			// ドアのヒンジ部分を軸に回転させるための処理
			// ドアの幅の半分の値（オフセット）
			float doorHalfWidth = 3.0f * worldTransform_.scale_.x;

			// 回転に応じた位置補正（回転前の位置に戻す）
			Vector3 offsetPosition = {};

			// 初期回転角度を考慮した計算
			float currentAngle = worldTransform_.rotation_.y;
			float startAngle = openAngle_; // 初期角度
			float rotationAmount = currentAngle - startAngle; // 実際の回転量

			if (normal_ == 0) {
				// Z軸方向を向いているドア（デフォルト）
				offsetPosition.x = doorHalfWidth * (1.0f - cosf(rotationAmount));
				offsetPosition.z = doorHalfWidth * sinf(rotationAmount);
			}
			else if (normal_ == 90) {
				// X軸方向を向いているドア（90度回転）
				offsetPosition.z = -doorHalfWidth * (1.0f - cosf(rotationAmount));
				offsetPosition.x = doorHalfWidth * sinf(rotationAmount);
			}
			else if (normal_ == 180) {
				// Z軸の逆方向を向いているドア（180度回転）
				offsetPosition.x = -doorHalfWidth * (1.0f - cosf(rotationAmount));
				offsetPosition.z = -doorHalfWidth * sinf(rotationAmount);
			}
			else if (normal_ == 270) {
				// X軸の逆方向を向いているドア（270度回転）
				offsetPosition.z = doorHalfWidth * (1.0f - cosf(rotationAmount));
				offsetPosition.x = -doorHalfWidth * sinf(rotationAmount);
			}

			// 位置を更新
			worldTransform_.translation_.x = position_.x + offsetPosition.x;
			worldTransform_.translation_.y = position_.y + offsetPosition.y;
			worldTransform_.translation_.z = position_.z + offsetPosition.z;
		}
		else {
			// 従来の回転処理（約90度固定）
			// 無効化してカスタム回転に統合
			useCustomRotation_ = true;
			targetRotation_ = 90.0f; // デフォルトの90度回転
			rotationSpeed_ = 2.5f;  // 当初の回転速度に相当
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
	
	// 回転データを表示
	ImGui::Text("Door ID: %d", doorID_);
	ImGui::Text("Using Custom Rotation: %s", useCustomRotation_ ? "Yes" : "No");
	
	if (useCustomRotation_) {
		ImGui::Text("Initial Rotation: %.2f degrees", initialRotation_);
		ImGui::Text("Target Rotation: %.2f degrees", targetRotation_);
		ImGui::Text("Rotation Speed: %.2f degrees/frame", rotationSpeed_);
		
		// 現在の回転角度（ラジアンから度数法に変換）
		float currentDegrees = worldTransform_.rotation_.y * (180.0f / static_cast<float>(M_PI));
		ImGui::Text("Current Rotation: %.2f degrees", currentDegrees);
		
		// アニメーション情報
		ImGui::Text("Is Animating: %s", isAnimating_ ? "Yes" : "No");
	}
	ImGui::End();
#endif
}

void Door::Draw() { model_->Draw(worldTransform_); }

AABB Door::GetAABB() {
	// ドアのローカル座標：
	//   min = (-3, 0, -1), max = (3, 6, 1)
	//   中心 = (0, 3, 0)
	//   半サイズ(=halfExtents) = (3, 3, 1)
	Vector3 localCenter = { 0.0f, 3.0f, 0.0f };
	Vector3 halfExtents = { 3.0f, 3.0f, 1.0f };

	// normal_値に応じて幅と奥行きを調整
	if (normal_ == 90 || normal_ == 270) {
		// X軸方向の場合は幅と奥行きを入れ替え
		halfExtents = { 1.0f, 3.0f, 3.0f };
	}

	// スケールを要素ごとに乗算
	Vector3 scaledCenter = { localCenter.x * worldTransform_.scale_.x, localCenter.y * worldTransform_.scale_.y, localCenter.z * worldTransform_.scale_.z };
	Vector3 scaledHalfExtents = { halfExtents.x * worldTransform_.scale_.x, halfExtents.y * worldTransform_.scale_.y, halfExtents.z * worldTransform_.scale_.z };

	// ドアのワールド中心 = ピボット位置(translation_) + scaledCenter
	Vector3 doorCenter = { worldTransform_.translation_.x + scaledCenter.x, worldTransform_.translation_.y + scaledCenter.y, worldTransform_.translation_.z + scaledCenter.z };

	// AABBを計算
	AABB doorAABB;
	doorAABB.min = { doorCenter.x - scaledHalfExtents.x, doorCenter.y - scaledHalfExtents.y, doorCenter.z - scaledHalfExtents.z };
	doorAABB.max = { doorCenter.x + scaledHalfExtents.x, doorCenter.y + scaledHalfExtents.y, doorCenter.z + scaledHalfExtents.z };

	// 当たり判定を確実にするため、開いている場合でも縮小率を小さくする
	if (isDoorOpened_) {
		// ドアが開いた状態ではAABBを少しだけ縮小する
		float reductionFactor = 0.8f; // 縮小率を大きくする（0.8は20%だけ縮小）
		Vector3 doorSize = { doorAABB.max.x - doorAABB.min.x, doorAABB.max.y - doorAABB.min.y, doorAABB.max.z - doorAABB.min.z };
		Vector3 reduction = { doorSize.x * (1.0f - reductionFactor) * 0.5f, 0.0f, doorSize.z * (1.0f - reductionFactor) * 0.5f };
		
		doorAABB.min.x += reduction.x;
		doorAABB.max.x -= reduction.x;
		doorAABB.min.z += reduction.z;
		doorAABB.max.z -= reduction.z;
	} else {
		// ドアが閉じている場合、確実に当たるように少し拡大
		// 特にプレイヤーとの小さな距離でも確実に当たるようにする
		float expansionFactor = 1.1f; // 10%拡大
		Vector3 doorSize = { doorAABB.max.x - doorAABB.min.x, doorAABB.max.y - doorAABB.min.y, doorAABB.max.z - doorAABB.min.z };
		Vector3 expansion = { doorSize.x * (expansionFactor - 1.0f) * 0.5f, 0.0f, doorSize.z * (expansionFactor - 1.0f) * 0.5f };
		
		doorAABB.min.x -= expansion.x;
		doorAABB.max.x += expansion.x;
		doorAABB.min.z -= expansion.z;
		doorAABB.max.z += expansion.z;
	}
	
	// デバッグ用に当たり判定の領域を表示
#ifdef _DEBUG
	// 当たり判定のAABBサイズを計算
	Vector3 aabbSize = {
		doorAABB.max.x - doorAABB.min.x,
		doorAABB.max.y - doorAABB.min.y,
		doorAABB.max.z - doorAABB.min.z
	};
	ImGui::Text("AABB Size: %.2f, %.2f, %.2f", aabbSize.x, aabbSize.y, aabbSize.z);
#endif

	return doorAABB;
}