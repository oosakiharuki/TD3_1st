#pragma once
#include "Object3d.h"
#include "AABB.h"
#include "CannonEnemy.h"
#include "Door.h"
#include "GhostEnemy.h"
#include "Key.h"
#include "Player.h"
#include "MoveTile.h"
#include <sstream>
#include <string>
#include <vector>

class StageManager {
public:
	StageManager();
	~StageManager();

	// 初期化
	void Initialize(uint32_t textureHandle);

	// 更新
	void Update();

	// 描画
	void Draw();

	// 現在のステージ番号を取得
	int GetCurrentStage() const { return currentStage_; }

	// ステージ関連メソッド
	void InitializeStage(int stageNumber);
	void TransitionToStage(int stageNumber);

	// プレイヤーの参照を設定/取得
	void SetPlayer(Player* player) { player_ = player; }
	Player* GetPlayer() const { return player_; }

	// 敵リストへのアクセス
	const std::vector<GhostEnemy*>& GetEnemyList() const { return ghostEnemyList_; }

	// 障害物リストへのアクセス
	const std::vector<std::vector<AABB>>& GetObstacleList() const { return allObstacles_; }

	// キャノン敵へのアクセス
	CannonEnemy* GetCannonEnemy() const { return cannonEnemy_; }

private:
	// ステージ地形の読み込みと生成
	void LoadStage(const std::string& objFile);
	void UpdateStageAABB();

	void AddObstacle(const Vector3& min, const Vector3& max);

	// プレイヤーの位置をリセット
	void ResetPlayerPosition(int stageNumber);

	// リソース関連
	uint32_t textureHandle_ = 0;
	Object3d* stageModel_ = nullptr;

	// オブジェクト管理
	Player* player_ = nullptr;
	std::vector<GhostEnemy*> ghostEnemyList_;
	CannonEnemy* cannonEnemy_ = nullptr;
	//Ground* ground_ = nullptr;
	Key* key_ = nullptr;
	Door* door_ = nullptr;

	// ステージデータ
	std::vector<std::vector<AABB>> allObstacles_;
	std::stringstream stageCommand_;

	// 移動床
	std::vector<MoveTile*> moveTiles_;

	// ステージ状態
	int currentStage_ = 2;              // 現在のステージ番号（1: 第1ステージ、2: 第2ステージ）
	bool isStageTransitioning_ = false; // ステージ遷移中フラグ
	float transitionTimer_ = 0.0f;      // 遷移タイマー
	float transitionDuration_ = 1.0f;   // 遷移にかかる時間
};