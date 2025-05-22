#pragma once
#include "WorldTransform.h"
#include "Object3d.h"
#include "AABB.h"
#include "collision.h"

#include "MyMath.h"

#include "Bom.h"
#include "Input.h"
#include "Audio.h"
#include "Block.h"

class Player;
class Particle;

class CannonEnemy {
public:
	CannonEnemy();

	~CannonEnemy();

	void Init();
	void Update();
	void Draw();
	void DrawP();
	
	void Fire();
	void PlayerFire();

	void SetObstacleList(const std::vector<AABB>& obstacles);

	void AddObstacle(const AABB& obstacle);

	void SetPlayer(Player* player) { player_ = player; }
	AABB GetAABB();

	void SetParent(const WorldTransform* parent) { worldTransform_.parent_ = parent; }
	void ContralPlayer();
	void ReMove(const Vector3& position_);
	bool GetPlayerCtrl() const { return isPlayer; }

	std::list<Bom*> GetBom() { return bullets_; }

	// 位置を設定するメソッドを追加
	void SetPosition(const Vector3& pos) {
		position = pos;
		worldTransform_.translation_ = pos;
		//リスポーン地点設定
		RespownPosition = pos;
	}
	
	// 壊せるブロックのリストを設定
	void SetBlocks(const std::vector<Block*>& blocks) { blocks_ = blocks; }

private:
	WorldTransform worldTransform_; // Fix the error by ensuring the type is defined
	Object3d* model_ = nullptr;
	Vector3 position = { 0, 5, 30 };
	Vector3 RespownPosition; //リスポーン地点
	// サウンド関連
	Audio* audio_ = nullptr;
	// 複数の爆発音サウンドデータ（連射用）
	static const int MAX_BOM_SOUNDS = 5;
	SoundData bomSounds_[MAX_BOM_SOUNDS];
	int currentBomSoundIndex_ = 0;
	bool onGround_ = true;
	float velocityY_ = 0.0f;

	// 障害物リスト
	std::vector<AABB> obstacleList_;
	
	// 破壊可能なブロックのリスト
	std::vector<Block*> blocks_;

	XINPUT_STATE state, preState;
	const float speed = 0.2f;

	// 移動
	bool Normal = true;
	float timer = 0.0f;
	const float corveTime = 1.0f;

	// スタン
	bool isStan = false;
	float timerS = 0.0f;
	const float stanTime = 3.0f;

	bool isPlayer = false;

	// 攻撃関連のパラメータ
	float attackRadius = 30.0f;     // 攻撃検知範囲（これを小さくすると検知範囲が縮小）
	float fireTimer = 3.0f;         // 現在の発射タイマー
	float fireInterval = 3.0f;      // 発射間隔（秒）- 3秒に変更

	float animertion = 0.0f;

	std::list<Bom*> bullets_;
	Player* player_ = nullptr;

	Particle* particleMove_ = nullptr;
};