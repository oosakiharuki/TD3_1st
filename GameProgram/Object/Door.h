#pragma once
#include "AABB.h"
#include "Key.h"
#include "Player.h"
#include "../../Engine/audio/Audio.h"
#include <vector>
#include <cmath> // M_PIを使用するために追加

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
		// 位置を初期化するときはアニメーション状態をリセット
		openAngle_ = 0.0f;
		isAnimating_ = false;
		isDoorOpened_ = false;
		soundPlayed_ = false; // サウンド再生状態もリセット
		
		// モデルが初期化されていれば、回転と位置オフセットを再計算
		if (model_) {
			// X、Y、Z軸の回転を設定
			worldTransform_.rotation_.x = rotationX_ * (3.14159265359f / 180.0f);
			worldTransform_.rotation_.y = rotationY_ * (3.14159265359f / 180.0f);
			worldTransform_.rotation_.z = rotationZ_ * (3.14159265359f / 180.0f);
			
			SetupPivotOffset();
			worldTransform_.UpdateMatrix();
		}
	}

	void SetRotateX(const float& rotateX) {
		// X軸回転値を保存
		rotationX_ = rotateX;
		
		// 初期化後であれば回転を適用
		if (model_) {
			// 回転を適用
			worldTransform_.rotation_.x = rotationX_ * (3.14159265359f / 180.0f); // 度からラジアンに変換
			
			// 行列を更新
			worldTransform_.UpdateMatrix();
		}
	}

	void SetRotateY(const float& rotateY) {
		// Y軸回転値を保存
		rotationY_ = rotateY;
		
		// 初期化後であれば回転を適用
		if (model_) {
			// 回転を適用
			worldTransform_.rotation_.y = rotationY_ * (3.14159265359f / 180.0f); // 度からラジアンに変換
			
			// 回転中心を再計算
			SetupPivotOffset();
			
			// 行列を更新
			worldTransform_.UpdateMatrix();
		}
	}

	void SetRotateZ(const float& rotateZ) {
		// Z軸回転値を保存
		rotationZ_ = rotateZ;
		
		// 初期化後であれば回転を適用
		if (model_) {
			// 回転を適用
			worldTransform_.rotation_.z = rotationZ_ * (3.14159265359f / 180.0f); // 度からラジアンに変換
			
			// 行列を更新
			worldTransform_.UpdateMatrix();
		}
	}

	// 現在の回転値を取得するメソッド
	Vector3 GetRotation() const {
		return { rotationX_, rotationY_, rotationZ_ };
	}

	void SetSize(const Vector3& size) {
		worldTransform_.scale_ = size;
	}

private:
	WorldTransform worldTransform_;
	Object3d* model_ = nullptr;
	Vector3 position_ = {1.5f, 0.0f, 48.592f}; // デフォルト位置（CSVから上書き可能）
	
	// ドアの回転中心を端（ヒンジ側）に設定する関数
	void SetupPivotOffset();

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

	float rotationX_ = 0.0f;
	float rotationY_ = 0.0f;
	float rotationZ_ = 0.0f;
};