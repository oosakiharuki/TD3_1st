#pragma once
#include "AABB.h"
#include "Key.h"
#include "Player.h"
#include "../../Engine/audio/Audio.h"
#include <vector>
#include <map>
#include <tuple>
#include <string>

class Door {
public:
	Door();
	~Door();

	// 初期化
	void Init();

	// 更新
	void Update();

	// 描画
	void Draw();

	// プレイヤーとキーの参照を設定
	void SetPlayer(Player* player) { player_ = player; }

	// 単一のキーを追加（後方互換性のため残す）
	void SetKey(Key* key) {
		if (key) {
			keys_.push_back(key);
		}
	}

	// 複数のキーをセット
	void SetKeys(const std::vector<Key*>& keys) { keys_ = keys; }

	// ドアが開いたかどうか
	bool IsDoorOpened() const { return isDoorOpened_; }

	// ドアに触れたかどうか
	bool IsDoorTouched() const { return isDoorTouched_; }

	// 必要なキーの総数を設定
	void SetRequiredKeyCount(int count) { requiredKeyCount_ = count; }

	// AABBを取得
	AABB GetAABB();

	// 位置を設定（CSVから読み込んだ位置に合わせるため）
	void SetPosition(const Vector3& position) {
		position_ = position;
		worldTransform_.translation_ = position;

		// 引き続きnormal_値に応じて初期回転を設定
		// モデルの向きを維持
		float normalRad = 0.0f;
		if (normal_ == 90) {
			normalRad = static_cast<float>(3.14159265358979323846) / 2.0f; // 90度
		}
		else if (normal_ == 180) {
			normalRad = static_cast<float>(3.14159265358979323846);     // 180度
		}
		else if (normal_ == 270) {
			normalRad = -static_cast<float>(3.14159265358979323846) / 2.0f; // 270度 (-90度)
		}
		
		// 初期角度を指定
		worldTransform_.rotation_.y = normalRad;
		openAngle_ = normalRad;

		// アニメーション状態をリセット
		isAnimating_ = false;
		isDoorOpened_ = false;
		soundPlayed_ = false; // サウンド再生状態もリセット
	}

	// 回転設定メソッド
	void SetRotateX(const float& rotateX) {
		worldTransform_.rotation_.x = rotateX * (static_cast<float>(3.14159265358979323846) / 180.0f);
	}

	void SetRotateY(const float& rotateY) {
		normal_ = rotateY;
		worldTransform_.rotation_.y = rotateY * (static_cast<float>(3.14159265358979323846) / 180.0f);
	}

	void SetRotateZ(const float& rotateZ) {
		worldTransform_.rotation_.z = rotateZ * (static_cast<float>(3.14159265358979323846) / 180.0f);
	}

	// 現在の回転を取得
	Vector3 GetRotation() const {
		return {
			worldTransform_.rotation_.x * (180.0f / static_cast<float>(3.14159265358979323846)),
			worldTransform_.rotation_.y * (180.0f / static_cast<float>(3.14159265358979323846)),
			worldTransform_.rotation_.z * (180.0f / static_cast<float>(3.14159265358979323846))
		};
	}

	void SetSize(const Vector3& size) {
		worldTransform_.scale_ = size;
	}

	// 回転設定をCSVから読み込む
	static bool LoadRotationsFromCSV(const std::string& filePath);

	// ドアのIDを設定
	void SetDoorID(int id) { doorID_ = id; }

	// ドアの開閉角度と速度を直接設定
	void SetDoorOpenParams(float openAngle, float speed) {
		targetRotation_ = openAngle;
		rotationSpeed_ = speed;
		useCustomRotation_ = true;
	}

	// MapLoader用のメソッド
	Vector3 GetTranslation() const { return worldTransform_.translation_; }
	void SetTranslation(const Vector3& translation) { 
		worldTransform_.translation_ = translation;
		position_ = translation;
	}
	Vector3 GetScale() const { return worldTransform_.scale_; }
	void SetScale(const Vector3& scale) { worldTransform_.scale_ = scale; }
	void SetRotation(const Vector3& rotation) { worldTransform_.rotation_ = rotation; }

private:
	WorldTransform worldTransform_;
	Object3d* model_ = nullptr;
	Vector3 position_ = { 1.5f, 0.0f, 48.592f }; // デフォルト位置（CSVから上書き可能）

	// サウンド関連
	SoundData doorOpenSound_;
	bool soundPlayed_ = false;

	// 参照
	Player* player_ = nullptr;
	std::vector<Key*> keys_; // 複数のキーを保持

	// ドア状態フラグ
	bool isDoorTouched_ = false; // ドアに触れたフラグ
	bool isDoorOpened_ = false;  // ドアが開いたフラグ

	// 必要なキーの数（デフォルトは1）
	int requiredKeyCount_ = 1;

	// 開閉アニメーション用変数
	float openAngle_ = 0.0f;
	bool isAnimating_ = false;

	// すべてのキーが取得されているかチェック
	bool AreAllKeysObtained() const;

	float normal_ = 0.0f;

	// 回転データ管理用
	int doorID_ = -1; // ドアのID（-1は未設定）
	float initialRotation_ = 0.0f;  // 初期回転角度（度数法）
	float targetRotation_ = 90.0f;  // 目標回転角度（度数法）
	float rotationSpeed_ = 2.0f;    // 回転速度（度数法/フレーム）
	bool useCustomRotation_ = false; // カスタム回転パラメータを使用するかどうか

	// 全ドアの回転データを保持する静的コンテナ
	static std::map<int, std::tuple<float, float, float>> doorRotations_; // doorID -> (initialRotation, targetRotation, rotationSpeed)
};