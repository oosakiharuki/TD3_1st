#pragma once
#include "AABB.h"
#include "Collision.h"
#include "WorldTransform.h"
#include "Object3d.h"
#include "Input.h"
#include <vector>
#include "Audio.h"
#include "Block.h"

class Player;      // 前方宣言
class SpringEnemy; // 前方宣言

class SpringEnemy {
public:
	SpringEnemy();
	~SpringEnemy();

	void Init();
	void Update();
	void Draw();

	// 障害物処理
	void SetObstacleList(const std::vector<AABB>& obstacles);
	void AddObstacle(const AABB& obstacle);
	void ClearObstacleList();

	// 位置設定
	void SetPosition(const Vector3& pos);
	
	// エディター用のメソッド
	Vector3 GetPosition() const { return position; }

	// 衝突判定用AABB
	AABB GetAABB() const;

	// プレイヤー参照
	void SetPlayer(Player* player) { player_ = player; }

	// スプリング専用メソッド
	float GetJumpBoost() const { return jumpBoostFactor; }
	void Compress();

	// プレイヤー操作メソッド（Enemyと同様）
	void SetParent(const WorldTransform* parent) { worldTransform_.parent_ = parent; }
	void ContralPlayer();
	void ReMove(const Vector3& position_);
	bool GetPlayerCtrl() { return isPlayer; }
	
	bool IsAttacking() const { return isAttacking_; } // 状態取得
	
	// 壊せるブロックのリストを設定
	void SetBlocks(const std::vector<Block*>& blocks) { blocks_ = blocks; }

private:
	WorldTransform worldTransform_;
	Object3d* model_ = nullptr;
	Vector3 position = {0, 0, -20};
	Vector3 RespownPosition; //リスポーン地点
	bool onGround_ = true;
	float velocityY_ = 0.0f;

	// 障害物リスト
	std::vector<AABB> obstacleList_;

	// 破壊可能なブロックのリスト
	std::vector<Block*> blocks_;

	// プレイヤー参照
	Player* player_ = nullptr;

	// スプリングのプロパティ
	float jumpBoostFactor = 1.2f;  // ジャンプブースト
	bool isCompressed = false;     // バネの見た目状態
	float compressionTimer = 0.0f; // アニメーション用タイマー
	float originalScaleY = 1.0f;   // 元のスケール（復元用）

	// プレイヤー操作（Enemyと同様）
	bool isPlayer = false;
	bool isStan = false;
	float timerS = 0.0f;
	const float stanTime = 3.0f;

	// 音声関連
	Audio* audio_ = nullptr;
	SoundData BaneSound;
	int springSoundHandle_ = 0;
	int springSoundID_ = -1;

	// デバッグ用
	XINPUT_STATE state, preState;

	float attackRadius_ = 2.0f;
	bool isAttacking_ = false;
	float attackTimer_ = 0.0f;
	const float attackDuration_ = 0.5f;  // 攻撃時間（秒）

	float cooldownTimer_ = 0.0f;
	const float attackCooldown_ = 2.0f;  // クールタイム（秒）

	float rotationSpeed_ = 10.0f; // 回転速度（度/フレーム）
};
