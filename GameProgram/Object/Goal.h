#pragma once
#include "2d/Sprite.h"
#include "3d/Camera.h"
#include "3d/Object3d.h"
#include "3d/WorldTransform.h"
#include "AABB.h"


class Goal {
public:
	Goal();
	~Goal();

	void Init();
	void Update();
	void Draw();
	void Text();

	void OnCollision();
	bool IsClear() const { return isClear; }

	AABB GetAABB();

	// 位置を設定するメソッドを追加
	void SetPosition(const Vector3& position) {
		worldTransform_.translation_ = position;
		worldTransform_.UpdateMatrix();
	}

private:

	WorldTransform worldTransform_;
	Object3d* model_ = nullptr;

	uint32_t textureHandle = 0;
	Sprite* sprite = nullptr;

	bool isClear = false;
};