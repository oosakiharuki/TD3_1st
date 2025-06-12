#pragma once
#include "2d/Sprite.h"
#include "3d/Camera.h"
#include "3d/Object3d.h"
#include "3d/WorldTransform.h"
#include "AABB.h"
#include "Audio.h"
#include "Particle.h"


class Goal {
public:
	Goal();
	~Goal();

	void Init();
	void Update();
	void Draw();
	void DrawP(); // パーティクル描画
	void Text();

	void OnCollision();
	bool IsClear() const { return isClear; }

	AABB GetAABB();

	// 位置を設定するメソッドを追加
	void SetPosition(const Vector3& position) {
		worldTransform_.translation_ = position;
		baseY_ = position.y; // ベースY座標を保存
		worldTransform_.UpdateMatrix();
	}

	// MapLoader用のメソッド
	Object3d* GetObject() { return model_; }
	WorldTransform& GetWorldTransform() { return worldTransform_; }

private:

	WorldTransform worldTransform_;
	Object3d* model_ = nullptr;

	uint32_t textureHandle = 0;
	Sprite* sprite = nullptr;

	bool isClear = false;

	// オーディオ
	Audio* audio_ = nullptr;
	SoundData clearSound_;

	// パーティクル
	Particle* particleCelebration_ = nullptr;

	// アニメーション用
	float floatingTime_ = 0.0f;
	float baseY_ = 0.0f;
	float celebrationTimer_ = 0.0f;
	float particleSpawnTimer_ = 0.0f;
};