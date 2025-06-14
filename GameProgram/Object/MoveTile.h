#pragma once
#include "AABB.h"
#include "Key.h"
#include "Player.h"
#include <vector>

class MoveTile {
public:

	MoveTile();
	~MoveTile();

	// 初期化
	void Init();

	// 更新
	void Update();

	// 描画
	void Draw();

	// プレイヤーとTileの参照を設定
	void SetPlayer(Player* player) { player_ = player; }

	// AABBを取得
	AABB GetAABB() const;

	// 位置を設定（CSVから読み込んだ位置に合わせるため）
	void SetPosition(const Vector3& position) {
		position_ = position;
		worldTransform_.translation_ = position;
	}

	// 移動速度を設定
	void SetMoveSpeed(float speed) { moveSpeed_ = speed; }

	// 移動範囲を設定
	void SetMoveRange(float range) { moveRange_ = range; }

	// 初期Y座標を設定
	void SetInitialY(float y) { initialY_ = y; }

	// Getter メソッド
	float GetSpeed() const { return moveSpeed_; }
	float GetRange() const { return moveRange_; }
	float GetInitialY() const { return initialY_; }
	Object3d* GetObject() { return model_; }
	WorldTransform& GetWorldTransform() { return worldTransform_; }

	// Setter メソッド（MapLoader.cppで使用）
	void SetSpeed(float speed) { moveSpeed_ = speed; }
	void SetRange(float range) { moveRange_ = range; }

	// プリセット情報
	void SetIsCustom(bool isCustom) { isCustom_ = isCustom; }
	bool IsCustom() const { return isCustom_; }

private:

	WorldTransform worldTransform_;
	Object3d* model_ = nullptr;
	Vector3 position_ = { 1.5f, 0.0f, 48.592f };

	Player* player_ = nullptr;

	// 上下移動のためのメンバ変数
	float moveSpeed_ = 1.0f;
	float moveRange_ = 15.0f;
	float initialY_ = 92;
	bool movingUp_ = true; // 上昇中か下降中かを示すフラグ
	int frameCount_ = 0; // フレームカウント（初期値0）
	bool isCustom_ = false; // カスタム設定かどうか
};