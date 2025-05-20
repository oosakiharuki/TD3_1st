#pragma once
#include "WorldTransform.h"
#include "Object3d.h"
#include "AABB.h"
#include "collision.h"

#include "Mymath.h"
#include "Audio.h"

class Bom {
public:
	Bom();
	~Bom();
	void Init(Vector3 position, Vector3 velocity);
	void Update();
	void Draw();
	bool IsDaed() { return isDead; }

	AABB GetAABB();
	void OnCollision();

	// サウンド関連の追加メソッド
	void SetSoundData(SoundData* soundData);
	Audio* GetAudio() const { return audio_; }

	Vector3 GetWorldPosition() { return worldTransform_.translation_; }

private:
	WorldTransform worldTransform_;
	Object3d* model_ = nullptr;
	Vector3 position_;
	bool onGround_ = true;

	// サウンド関連
	Audio* audio_ = nullptr; // オーディオインスタンス
	SoundData* soundData_ = nullptr; // 再生中のサウンドデータへのポインタ

	Vector3 velocity_;
	float deadTimer = 3.0f;
	bool isDead = false;
	// 表示フラグを追加
	bool isVisible = true;
};