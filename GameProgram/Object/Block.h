#pragma once
#include "AABB.h"
#include "WorldTransform.h"
#include "Object3d.h"
#include "Particle.h"

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

private:
	WorldTransform worldTransform;
	Camera* viewProjection_ = nullptr;
	Object3d* model_ = nullptr;
	bool isActive_ = true; // ブロックが有効かどうか
	uint32_t texturehandle_ = 0;

	Vector3 size_;
	uint32_t hp = 3;
	Particle* particle = nullptr;
	Vector3 particlePosition = { 0,0,0 };
};