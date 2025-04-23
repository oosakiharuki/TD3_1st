#pragma once
#include "AABB.h"
#include "Block.h"
#include "CameraController.h"
#include "CannonEnemy.h"
#include "Collision.h"
#include "GhostBlock.h"
#include "Goal.h"
#include "MyMath.h"
#include "SpringEnemy.h"
#include <vector>
#include "Audio.h"

#include "Particle.h"

class GhostEnemy;

class Player {
public:
	Player();
	~Player();

	void Init(Camera* camera);
	void Update();
	void Draw();

	AABB GetAABB() { return playerAABB; }
	void IsOnEnemy(bool set) { onEnemy = set; }
	bool GetIsTransfar() { return isTransfar; }

	void GetEnemyHead(AABB aabb) { enemyAABB = aabb; }

	const WorldTransform* GetWorld() { return &worldTransform_; }
	Vector3 GetWorldPosition() { return position; }

	bool GetEnemyContral() { return EnemyContral; }
	void SetEnemyContral(bool anser) {
		isTransfar = false;
		velocity.y = 0.0f;
		EnemyContral = anser;
	}

	void SetObstacleList(const std::vector<AABB>& obstacles);
	void AddObstacle(const AABB& obstacle);

	void SetGhostEnemies(const std::vector<GhostEnemy*>& enemies);

	std::vector<GhostEnemy*> ghostEnemies_;

	void EnemyHead() { onEnemy = true; }

	void SetPosition(const Vector3& position);

	// 初期位置を設定・取得するメソッド
	void SetInitialPosition(const Vector3& initialPos) { initialPosition = initialPos; }
	Vector3 GetInitialPosition() const { return initialPosition; }

	// 重複宣言を削除し、ここに1つだけ残す
	void SetCannon(CannonEnemy* cannon); // 後方互換性のため残す
	void SetCannonEnemies(const std::vector<CannonEnemy*>& cannons);


	void OnCollisions();

	void ResolveCollisionWithDoor(const AABB& aabb) { doorAABB = aabb; }
	void ResolveCollisionWithTile(const AABB& aabb) { moveTileAABBs.push_back(aabb); }
	void SetOpenDoor(bool isOpen) { isOpenDoor = isOpen; }

	void SetSpringEnemies(const std::vector<SpringEnemy*>& springEnemies);
	// ここに重複していた宣言を削除
	void CheckCollisionWithSprings();
	void SetBlocks(const std::vector<Block*> blocks) { blocks_ = blocks; }

	void SetGhostBlocks(const std::vector<GhostBlock*> blocks) { ghostBlocks_ = blocks; }

	void SetGoal(Goal* goal) { goal_ = goal; }
	void CheckCollisionWithGoal();

	enum class State {
		Normal, // 通常状態
		Bomb,   // ブロックを壊せる状態
		Ghost   // ブロックをすり抜ける状態
	};

	void DrawUI();

	void CheckCollision();
	void SetState(State newState);
	void ClearObstacleList();
	void CheckDamage();

	void TakeDamage();
	// 指定されたダメージを受ける関数を追加
	void TakeDamage(int damageAmount);
	int GetHp() { return hp; }

	void DrawP();

private:
	Vector3 position = { 0, 10, -10 };
	Vector3 initialPosition = { 0, 10, -10 }; // 初期位置保存用
	Vector3 velocity;
	Vector3 size = { 2, 2, 2 };
	Vector3 stop = { 0, 0, 0 };

	WorldTransform worldTransform_;
	Camera* camera_ = nullptr;
	Object3d* PlayerModel_ = nullptr;
	CameraController cameraController_;

	bool onGround_ = true;
	bool onEnemy;
	bool isTransfar = false;
	bool isDamage = false;
	bool EnemyContral = false;
	bool collisionEnemy = false;
	bool isOpenDoor = false;

	float velocityY_ = 0.0f;
	int hp = 200;
	float coolTime = 0.0f;
	float cameraPitch = 5.0f;
	float cameraYaw = 0.0f;
	const float speed = 0.25f;
	XINPUT_STATE state = {}, preState = {}; // 初期化を追加	
	AABB playerAABB;
	AABB enemyAABB;
	AABB doorAABB;
	std::vector<AABB> moveTileAABBs;

	// 落下判定用
	const float fallThreshold = -60.0f;
	const int fallDamage = 40; // 通常ダメージの2倍相当

	uint32_t textureHandle = 0;
	//GhostEnemy* ghostEnemy = nullptr;//乗っ取ったゴースト
	State currentState = State::Normal;
	std::vector<CannonEnemy*> cannonEnemies_; // 単一ポインタではなくベクトルに変更
	CannonEnemy* cannonEnemy = nullptr;


	Goal* goal_ = nullptr;

	std::vector<AABB> obstacleList_;
	std::vector<SpringEnemy*> springEnemies_;
	std::vector<Block*> blocks_;
	std::vector<GhostBlock*> ghostBlocks_;

	// 点滅関連の追加変数
	bool isFlashing = false;          // 点滅中かどうか
	float flashTimer = 0.0f;          // 点滅用タイマー
	bool isVisible = true;            // 現在表示中かどうか
	const float flashInterval = 0.1f; // 点滅間隔（秒）
	const float flashDuration = 1.0f; // 点滅継続時間（秒）

	// サウンド関連
	Audio* audio_ = nullptr;
	SoundData JumpSound_;
	SoundData SnapSound_;
	SoundData DamageSound_;
	SoundData FallSound_; // 落下音追加

	// 初期位置にリセットする関数
	void ResetToInitialPosition();
	// 落下チェック関数
	void CheckFallOut();


	Particle* particleMove_ = nullptr;
	Particle* particleTransfar_ = nullptr;

	float RotateY = 0.0f;
};