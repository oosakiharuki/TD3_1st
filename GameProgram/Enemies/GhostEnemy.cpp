#include "GhostEnemy.h"
#include "AABB.h"
#include "Collision.h"
#include "ImGuiManager.h"
#include <algorithm>
#include <iostream>

using namespace MyMath;

GhostEnemy::GhostEnemy() {}

GhostEnemy::~GhostEnemy() { delete model_; }

void GhostEnemy::Init() {
	worldTransform_.Initialize();
	// "cube" モデルを読み込み
	model_ = new Object3d();
	model_->Initialize();
	model_->SetModelFile("BlueGhost");
	worldTransform_.translation_ = position;
}

void GhostEnemy::SetObstacleList(const std::vector<AABB>& obstacles) { obstacleList_.insert(obstacleList_.end(), obstacles.begin(), obstacles.end()); }

void GhostEnemy::AddObstacle(const AABB& obstacle) { obstacleList_.push_back(obstacle); }

void GhostEnemy::SetPosition(const Vector3& pos) {
	position = pos;
	worldTransform_.translation_ = position;
}

void GhostEnemy::ClearObstacleList() {
	obstacleList_.clear(); // ✅ 障害物リストをクリア
}

void GhostEnemy::SetTarget(Player* target) {
	player_ = target; // プレイヤーをターゲットとして設定
}

void GhostEnemy::Update() {
	// 入力による移動
	// float moveSpeed = 0.0f;

	// const float moveSpeed_ = 0.02f;
	const float deltaTime = 1.0f / 60.0f;

	if (isStan) {
		timerS += deltaTime;
		if (timerS > stanTime) {
			isStan = false;
			timerS = 0.0f;
		}
	}

	if (!isPlayer) {

		// 重力処理
		float gravity = 0.01f;
		velocityY_ -= gravity;
		position.y += velocityY_;

		// プレイヤーのAABB作成（例：幅1.0, 高さ2.0, 奥行1.0）
		float halfW = 1.0f, halfH = 1.0f, halfD = 1.0f;
		AABB enemyAABB;
		enemyAABB.min = { position.x - halfW, position.y - halfH, position.z - halfD };
		enemyAABB.max = { position.x + halfW, position.y + halfH, position.z + halfD };

		// 反復的衝突解決（すり抜け防止のため、最大10回まで解決を試みる）
		const int maxIterations = 10;
		int iterations = 0;
		bool collisionOccurred = false;
		do {
			collisionOccurred = false;
			for (auto& obstacleAABB : obstacleList_) {
				if (IsCollisionAABB(enemyAABB, obstacleAABB)) {
					ResolveAABBCollision(enemyAABB, obstacleAABB, velocityY_, onGround_);
					collisionOccurred = true;
				}
				else {
					onGround_ = false;
				}
			}
			iterations++;
		} while (collisionOccurred && iterations < maxIterations);

		// 衝突解決後のAABB中心をプレイヤー座標に反映
		position.x = (enemyAABB.min.x + enemyAABB.max.x) * 0.5f;
		position.y = (enemyAABB.min.y + enemyAABB.max.y) * 0.5f;
		position.z = (enemyAABB.min.z + enemyAABB.max.z) * 0.5f;

		Vector3 move = worldTransform_.translation_;

		if (IsPlayerInChaseRadius()) {
			// プレイヤーへのベクトルを計算
			Vector3 playerWorldPosition = player_->GetWorldPosition();
			Vector3 enemyWorldPosition = GetWorldPosition();
			Vector3 toPlayer = Normalize(playerWorldPosition - enemyWorldPosition);

			Vector3 direction = playerWorldPosition - enemyWorldPosition;
			direction.y = 0; // Y軸回転のみなので高さは無視
			direction = Normalize(direction);

			// Y軸の回転角度を計算
			worldTransform_.rotation_.y = atan2(direction.x, -direction.z);

			// 一定速度でプレイヤーに向かうベクトルで設定
			velocity = toPlayer * kChaseSpeed;

			// Playerとの衝突判定
			if (CheckCollisionWithPlayer()) {
				// 衝突時の処理（移動を停止）
				velocity.x = 0;
				velocity.z = 0;
			}
			else {
				// 速度をそのまま適用
				worldTransform_.translation_.x += velocity.x;
				worldTransform_.translation_.z += velocity.z;
			}
		}
		worldTransform_.translation_.y = position.y;
	}
#ifdef _DEBUG
	ImGui::Begin("GhostEnemy");
	ImGui::DragFloat3("translate", &position.x);
	ImGui::End();
#endif
	worldTransform_.UpdateMatrix();
}

void GhostEnemy::Draw() {
	switch (colorType)
	{
	case ColorType::Blue:
		model_->Draw(worldTransform_);
		break;
	case ColorType::Green:
		model_->Draw(worldTransform_, "resource/Sprite/GreenGhost.png");
		break;
	case ColorType::Red:
		model_->Draw(worldTransform_, "resource/Sprite/RedGhost.png");
		break;
	default:
		break;
	}
}

// AABBを取得するメソッドを定義
AABB GhostEnemy::GetAABB() const {
	float halfW = 1.0f, halfH = 1.0f, halfD = 1.0f;
	AABB enemyAABB;
	enemyAABB.min = { worldTransform_.translation_.x - halfW, worldTransform_.translation_.y - halfH, worldTransform_.translation_.z - halfD };
	enemyAABB.max = { worldTransform_.translation_.x + halfW, worldTransform_.translation_.y + halfH, worldTransform_.translation_.z + halfD };
	return enemyAABB;
}

void GhostEnemy::ContralPlayer() {
	isPlayer = true;
	worldTransform_.translation_ = { 0, -2, 0 };
	worldTransform_.rotation_ = { 0, 3, 0 };
	if (player_) {
		player_->SetState(Player::State::Ghost);
	}
}

void GhostEnemy::ReMove(const Vector3& position_) {
	if (isPlayer) {
		position.x = position_.x;
		position.y = position_.y - 2;
		position.z = position_.z;
		timerS = 0.0f;
		isStan = true;
		isPlayer = false;
		worldTransform_.parent_ = nullptr;
		if (player_) {
			player_->SetState(Player::State::Normal);
		}
	}
}

Vector3 GhostEnemy::GetWorldPosition() {

	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得（ワールド座標）
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

bool GhostEnemy::CheckCollisionWithPlayer() {
	AABB playerAABB = player_->GetAABB();
	AABB enemyAABB = GetAABB();
	return IsCollisionAABB(playerAABB, enemyAABB);
}

void GhostEnemy::SetChaseRadius(float radius_) { chaseRadius_ = radius_; }

// プレイヤーが追尾範囲内にいるかどうかを判定
bool GhostEnemy::IsPlayerInChaseRadius() {
	Vector3 playerWorldPosition = player_->GetWorldPosition();
	Vector3 enemyWorldPosition = GetWorldPosition();
	float distance = Length(playerWorldPosition - enemyWorldPosition);
	return distance <= chaseRadius_;
}