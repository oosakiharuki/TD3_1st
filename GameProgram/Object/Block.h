#pragma once
#include "AABB.h"
#include "WorldTransform.h"
#include "Object3d.h"
#include "Particle.h"
#include "Audio.h"

class Block {
public:
	Block();
	~Block();
	void Init();
	void Update();
	void Draw();
	void DrawP();

	bool IsActive() const { return isActive_; }         // アクティブ状態を取得
	void SetActive(bool active) { isActive_ = active; } // アクティブ状態を設定

	AABB GetAABB() const; // AABBの取得

	// 位置を設定するメソッドを追加
	void SetPosition(const Vector3& pos);
	void SetSize(const Vector3& size);

	void OnCollision();
	void SetParticlePosition(Vector3 position) { particlePosition = position; }

	// HPを取得するメソッド
	uint32_t GetHP() const { return hp; }

	// MapLoader用のメソッド
	Vector3 GetTranslation() const { return worldTransform.translation_; }
	void SetTranslation(const Vector3& translation) { 
		worldTransform.translation_ = translation;
		originalPosition_ = translation;
	}
	Vector3 GetScale() const { return size_; }
	void SetScale(const Vector3& scale) { 
		size_ = scale;
		SetSize(scale);
	}

private:
	WorldTransform worldTransform;
	Camera* viewProjection_ = nullptr;
	Object3d* model_ = nullptr;
	bool isActive_ = true; // ブロックが有効かどうか
	uint32_t texturehandle_ = 0;

	Vector3 size_;
	Vector3 originalPosition_; // 元の位置（揺れエフェクト用）
	uint32_t hp = 3;
	uint32_t maxHP = 3; // 最大HP
	Particle* particle = nullptr;
	Vector3 particlePosition = { 0,0,0 };

	// ダメージエフェクト用
	float damageTimer_ = 0.0f;
	float scaleTimer_ = 0.0f;
	float shakeTimer_ = 0.0f;
	bool isDamaged_ = false;

	// エフェクト用のパラメータ（控えめに調整）
	const float DAMAGE_EFFECT_TIME = 0.3f;
	const float SCALE_EFFECT_TIME = 0.2f;
	const float SHAKE_EFFECT_TIME = 0.15f;
	float SHAKE_INTENSITY = 0.05f; // より控えめに
	float SCALE_INTENSITY = 1.1f; // より控えめに

	// オーディオ
	Audio* audio_ = nullptr;
	SoundData hitSound_;
	SoundData breakSound_;
};