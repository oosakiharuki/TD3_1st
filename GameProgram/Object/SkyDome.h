#pragma once
#include "3d/Object3d.h"
#include "3d/WorldTransform.h"
#include <assert.h>
#include <numbers>

class Skydome {

public:
	void Initialize();

	void Update();

	void Draw();

private:
	// ワールド変換データ
	WorldTransform worldTransform_;
	// モデル
	Object3d* model_ = nullptr;

	// モデルの移動速度や位置を格納する変数
	Vector3 velocity_; // 移動速度
};