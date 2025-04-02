#pragma once
#include "AABB.h"
#include "WorldTransform.h"
#include "Object3d.h"

class Block {
public:
	Block();
	~Block();
	void Init();
	void Update();
	void Draw();

	bool IsActive() const { return isActive_; }         // アクティブ状態を取得
	void SetActive(bool active) { isActive_ = active; } // アクティブ状態を設定

	AABB GetAABB() const; // AABBの取得

	// 位置を設定するメソッドを追加
	void SetPosition(const Vector3& pos);

private:
	WorldTransform worldTransform;
	Camera* viewProjection_ = nullptr;
	Object3d* model_ = nullptr;
	bool isActive_ = true; // ブロックが有効かどうか
	uint32_t texturehandle_ = 0;
};