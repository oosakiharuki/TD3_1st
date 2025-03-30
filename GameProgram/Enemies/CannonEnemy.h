#pragma once
#include "3d/WorldTransform.h"
#include "AABB.h"
#include "collision.h"

#include "Mymath.h"

#include "Bom.h"

class Player;

class CannonEnemy {
public:
	CannonEnemy();

	~CannonEnemy();

	void Init(Camera* camera);
	void Update();
	void Draw();

	void Fire();
	void PlayerFire();

	void SetObstacleList(const std::vector<AABB>& obstacles);

	void AddObstacle(const AABB& obstacle);

	// void SetPlayerAABB(AABB aabb) { playerAABB = aabb; }

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
	}

private:
	WorldTransform worldTransform_; // Fix the error by ensuring the type is defined
	Camera* camera_ = nullptr;
	Model* model_ = nullptr;
	Vector3 position = {0, 5, 30};
	bool onGround_ = true;
	float velocityY_ = 0.0f;

	// 障害物リスト
	std::vector<AABB> obstacleList_;

	// WorldTransform worldTransform;

	// Vector3 velocity;
	//  bool IsJump = false;

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

	float radius = 50.0f;
	float fireTimer = 2.0f;

	std::list<Bom*> bullets_;
	Player* player_ = nullptr;
};