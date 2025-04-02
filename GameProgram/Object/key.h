#pragma once
#include "AABB.h"
#include "Player.h"
#include "Audio.h"

class Key {
public:
	Key();
	~Key();

	// 初期化
	void Init();

	// 更新
	void Update();

	// 描画
	void Draw();

	// プレイヤーの参照を設定
	void SetPlayer(Player* player) { player_ = player; }

	// キーが取得されたかどうか
	bool IsKeyObtained() const { return isObtained_; }

	// キーのID（複数キーの識別用）
	void SetKeyID(int id) { keyID_ = id; }
	int GetKeyID() const { return keyID_; }

	// AABBを取得
	AABB GetAABB() const;

	// 位置を設定（CSVから読み込んだ位置に合わせるため）
	void SetPosition(const Vector3& position) {
		position_ = position;
		worldTransform_.translation_ = position;
	}
	Vector3 GetPosition() const { return position_; }
private:
	WorldTransform worldTransform_;
	Object3d* model_ = nullptr;
	Vector3 position_ = {3.499f, 9.510f, -47.592f}; // デフォルト位置（CSVから上書き可能）

	// プレイヤー参照
	Player* player_ = nullptr;

	// 鍵取得フラグ
	bool isObtained_ = false;

	// 鍵のID（複数の鍵を区別するため）
	int keyID_ = 0;
	Audio* keyGTAudio_ = nullptr;
	//int KeyAudioHandle_ = 0;
	int KeyGetAudio_ = 0;

	// 回転アニメーション用
	float rotationY_ = 0.0f;
};