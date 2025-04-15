#include "GhostEnemy.h"
#include "AABB.h"
#include "Collision.h"
#include "ImGuiManager.h"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace MyMath;

GhostEnemy::GhostEnemy() {
	// 乱数初期化（最初の実行時だけ）
	static bool randomInitialized = false;
	if (!randomInitialized) {
		std::srand(static_cast<unsigned int>(std::time(nullptr)));
		randomInitialized = true;
	}

	// 最初の目的地を設定
	SetNewRandomDestination();
}

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
	obstacleList_.clear();
}

void GhostEnemy::SetTarget(Player* target) {
	player_ = target;
}

void GhostEnemy::Update() {
	const float deltaTime = 1.0f / 60.0f;

	// スタン状態の処理
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

		// プレイヤーのAABB作成
		float halfW = 1.0f, halfH = 1.0f, halfD = 1.0f;
		AABB enemyAABB;
		enemyAABB.min = { position.x - halfW, position.y - halfH, position.z - halfD };
		enemyAABB.max = { position.x + halfW, position.y + halfH, position.z + halfD };

		// 反復的衝突解決
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

		// プレイヤーが追跡範囲内にいるか確認
		bool playerInRange = IsPlayerInChaseRadius();

		// 他のゴーストとの距離をチェック
		bool tooCloseToOtherGhost = false;
		Vector3 separationVector = { 0.0f, 0.0f, 0.0f };

		// お互いのゴーストの距離を計算し、近すぎる場合は離れる方向に移動する
		if (otherGhosts_ && !otherGhosts_->empty()) {
			for (auto* otherGhost : *otherGhosts_) {
				// 自分自身は無視
				if (otherGhost == this) continue;

				Vector3 otherPos = otherGhost->GetWorldTranslate();
				Vector3 toOther = otherPos - position;
				float distance = Length(toOther);

				// 一定距離以下に近づいている場合
				if (distance < minGhostDistance_) {
					tooCloseToOtherGhost = true;

					// 反対方向に向かうベクトルを追加（距離が近いほど強く）
					if (distance > 0.1f) { // ゼロ除算防止
						Vector3 awayVector = position - otherPos;
						awayVector = Normalize(awayVector);
						// 距離が近いほど強く離れるように
						float separationStrength = (minGhostDistance_ - distance) / minGhostDistance_;
						separationVector.x += awayVector.x * separationStrength;
						separationVector.z += awayVector.z * separationStrength;
					}
				}
			}

			// 分離ベクトルがゼロでなければ正規化
			if (Length(separationVector) > 0.001f) {
				separationVector = Normalize(separationVector);
				// 分離速度を適用
				separationVector.x *= kSeparationSpeed;
				separationVector.z *= kSeparationSpeed;
			}
		}

		// 行動の優先度
		// 1. プレイヤーが範囲内なら追跡
		// 2. 他のゴーストと近すぎたら離れる
		// 3. それ以外はランダム移動
		if (playerInRange) {
			// プレイヤー追跡モード
			Vector3 playerWorldPosition = player_->GetWorldPosition();
			Vector3 enemyWorldPosition = GetWorldPosition();
			Vector3 toPlayer = Normalize(playerWorldPosition - enemyWorldPosition);

			Vector3 direction = playerWorldPosition - enemyWorldPosition;
			direction.y = 0; // Y軸回転のみなので高さは無視
			direction = Normalize(direction);

			// Y軸の回転角度を計算
			worldTransform_.rotation_.y = atan2(direction.x, -direction.z);

			// 一定速度でプレイヤーに向かうベクトルを設定
			velocity = toPlayer * kChaseSpeed;

			// ゴースト同士が近すぎる場合、分離方向も考慮
			if (tooCloseToOtherGhost) {
				// 追跡と分離を混合（重みを調整可能）
				velocity.x = velocity.x * 0.6f + separationVector.x * 0.4f;
				velocity.z = velocity.z * 0.6f + separationVector.z * 0.4f;
			}

			// Playerとの衝突判定
			if (CheckCollisionWithPlayer()) {
				// 衝突時の処理（移動を停止）
				velocity.x = 0;
				velocity.z = 0;
			}
			else {
				// 速度を適用
				worldTransform_.translation_.x += velocity.x;
				worldTransform_.translation_.z += velocity.z;
			}

			// プレイヤー追跡中はランダム移動タイマーをリセット
			randomMoveTimer_ = 0.0f;
		}
		else if (tooCloseToOtherGhost) {
			// 他のゴーストとの距離調整が優先
			worldTransform_.translation_.x += separationVector.x;
			worldTransform_.translation_.z += separationVector.z;

			// 分離中も目的地に向かう方向を維持
			Vector3 toDestination = randomDestination_ - GetWorldPosition();
			toDestination.y = 0; // Y軸方向は無視

			if (Length(toDestination) > 0.001f) {
				Vector3 direction = Normalize(toDestination);
				worldTransform_.rotation_.y = atan2(direction.x, -direction.z);
			}
		}
		else {
			// ランダム移動モード
			UpdateRandomMovement(deltaTime);
		}

		// フィールド境界チェック
		EnforceFieldBoundaries();

		// すべての障害物との当たり判定を確認（プレイヤーが乗り移っていないとき）
		if (!isPlayer) {
			CheckCollisionWithObstacles();
		}


		CheckAndResolveGhostCollisions();

		worldTransform_.translation_.y = position.y;
	}

#ifdef _DEBUG
	ImGui::Begin("GhostEnemy");
	ImGui::DragFloat3("translate", &position.x);
	ImGui::DragFloat("Chase Radius", &chaseRadius_, 1.0f, 5.0f, 100.0f);
	ImGui::DragFloat("Min Ghost Distance", &minGhostDistance_, 0.5f, 1.0f, 20.0f);
	ImGui::DragFloat("Random Move Radius", &randomMoveRadius_, 5.0f, 10.0f, 200.0f);
	ImGui::DragFloat("Random Move Speed", &randomMoveSpeed_, 0.01f, 0.05f, 0.5f);
	ImGui::DragFloat("Destination Threshold", &destinationThreshold_, 0.1f, 1.0f, 10.0f);

	// フィールド境界のデバッグ表示
	ImGui::DragFloat3("Field Min", &fieldMin_.x, 1.0f);
	ImGui::DragFloat3("Field Max", &fieldMax_.x, 1.0f);

	// 目的地と移動タイマーの表示
	ImGui::Text("Destination: %.1f, %.1f, %.1f", randomDestination_.x, randomDestination_.y, randomDestination_.z);
	ImGui::Text("Move Timer: %.1f", randomMoveTimer_);

	// 状態の表示
	//ImGui::Text("State: %s", playerInRange ? "Chasing Player" : (tooCloseToOtherGhost ? "Avoiding Ghost" : "Random Move"));
	ImGui::Text("On Ground: %s", onGround_ ? "True" : "False");

	ImGui::End();
#endif

	worldTransform_.UpdateMatrix();
}

void GhostEnemy::UpdateRandomMovement(float deltaTime) {
	// 現在位置から目的地への方向ベクトル
	Vector3 currentPosition = GetWorldPosition();
	Vector3 toDestination = randomDestination_ - currentPosition;
	toDestination.y = 0; // Y軸方向は考慮しない（高さは重力で制御）

	// 目的地までの距離
	float distanceToDestination = Length(toDestination);

	// 目的地に十分近づいたか、移動タイマーが切れたら新しい目的地を設定
	if (distanceToDestination < destinationThreshold_ || randomMoveTimer_ > randomMoveDuration_) {
		SetNewRandomDestination();
		toDestination = randomDestination_ - currentPosition;
		toDestination.y = 0;
		distanceToDestination = Length(toDestination);
	}

	// 移動タイマーを更新
	randomMoveTimer_ += deltaTime;

	// 目的地方向の単位ベクトル
	Vector3 moveDirection = Normalize(toDestination);

	// 移動方向を向く
	worldTransform_.rotation_.y = atan2(moveDirection.x, -moveDirection.z);

	// 移動
	worldTransform_.translation_.x += moveDirection.x * randomMoveSpeed_;
	worldTransform_.translation_.z += moveDirection.z * randomMoveSpeed_;
}

void GhostEnemy::SetNewRandomDestination() {
	// 現在位置を基準に、ランダムな方向に一定半径内の点を目的地とする
	Vector3 currentPos = GetWorldPosition();

	// ランダムな角度を生成（0～2π）
	float randomAngle = static_cast<float>(std::rand()) / RAND_MAX * 2.0f * 3.14159265f;

	// ランダムな距離を生成（0～randomMoveRadius_）
	float randomDistance = static_cast<float>(std::rand()) / RAND_MAX * randomMoveRadius_;

	// 極座標から直交座標へ変換
	float offsetX = std::cos(randomAngle) * randomDistance;
	float offsetZ = std::sin(randomAngle) * randomDistance;

	// 新しい目的地を設定
	randomDestination_.x = currentPos.x + offsetX;
	randomDestination_.y = currentPos.y; // 高さは現在と同じ
	randomDestination_.z = currentPos.z + offsetZ;

	// フィールド内に収まるよう制限
	randomDestination_.x = std::clamp(randomDestination_.x, fieldMin_.x, fieldMax_.x);
	randomDestination_.z = std::clamp(randomDestination_.z, fieldMin_.z, fieldMax_.z);

	// タイマーリセット
	randomMoveTimer_ = 0.0f;

	// ランダムな移動時間を設定（5～10秒）
	randomMoveDuration_ = 5.0f + static_cast<float>(std::rand()) / RAND_MAX * 5.0f;
}

void GhostEnemy::EnforceFieldBoundaries() {
	// 現在位置がフィールド境界を超えていないか確認し、制限する
	Vector3 currentPos = worldTransform_.translation_;

	// X軸の制限
	if (currentPos.x < fieldMin_.x) {
		worldTransform_.translation_.x = fieldMin_.x;
		// 反発：フィールドの中心方向に向かうようにする
		if (randomDestination_.x < fieldMin_.x) {
			randomDestination_.x = fieldMin_.x + bounceBackDistance_;
		}
	}
	else if (currentPos.x > fieldMax_.x) {
		worldTransform_.translation_.x = fieldMax_.x;
		if (randomDestination_.x > fieldMax_.x) {
			randomDestination_.x = fieldMax_.x - bounceBackDistance_;
		}
	}

	// Z軸の制限
	if (currentPos.z < fieldMin_.z) {
		worldTransform_.translation_.z = fieldMin_.z;
		if (randomDestination_.z < fieldMin_.z) {
			randomDestination_.z = fieldMin_.z + bounceBackDistance_;
		}
	}
	else if (currentPos.z > fieldMax_.z) {
		worldTransform_.translation_.z = fieldMax_.z;
		if (randomDestination_.z > fieldMax_.z) {
			randomDestination_.z = fieldMax_.z - bounceBackDistance_;
		}
	}

	// フィールド境界付近にいる場合は、より中央に向かう新しい目的地を設定
	float borderThreshold = 5.0f; // 境界からこの距離以内だと中央に向かう

	bool nearBoundary =
		(currentPos.x < fieldMin_.x + borderThreshold) ||
		(currentPos.x > fieldMax_.x - borderThreshold) ||
		(currentPos.z < fieldMin_.z + borderThreshold) ||
		(currentPos.z > fieldMax_.z - borderThreshold);

	if (nearBoundary && randomMoveTimer_ > 3.0f) { // 一定時間は現在の目的地に向かう
		// フィールド中央付近に新しい目的地を設定
		Vector3 fieldCenter = {
			(fieldMin_.x + fieldMax_.x) * 0.5f,
			currentPos.y,
			(fieldMin_.z + fieldMax_.z) * 0.5f
		};

		// 中央に向かうベクトルを計算
		Vector3 toCenter = fieldCenter - currentPos;
		toCenter.y = 0.0f; // Y軸は無視

		if (Length(toCenter) > 0.001f) {
			toCenter = Normalize(toCenter);

			// 中央方向に目的地を設定（少しランダム性を加える）
			float randomOffset = 20.0f;
			randomDestination_.x = fieldCenter.x + (static_cast<float>(std::rand()) / RAND_MAX * randomOffset - randomOffset * 0.5f);
			randomDestination_.z = fieldCenter.z + (static_cast<float>(std::rand()) / RAND_MAX * randomOffset - randomOffset * 0.5f);

			// 目的地をフィールド内に制限
			randomDestination_.x = std::clamp(randomDestination_.x, fieldMin_.x + 5.0f, fieldMax_.x - 5.0f);
			randomDestination_.z = std::clamp(randomDestination_.z, fieldMin_.z + 5.0f, fieldMax_.z - 5.0f);

			randomMoveTimer_ = 0.0f;
		}
	}
}

void GhostEnemy::CheckCollisionWithObstacles() {
	// ゴーストのAABB
	AABB ghostAABB = GetAABB();

	// すべての障害物と衝突判定
	for (const auto& obstacleAABB : obstacleList_) {
		if (IsCollisionAABB(ghostAABB, obstacleAABB)) {
			// 衝突したら押し出し処理
			Vector3 overlap = GetOverlapAmount(ghostAABB, obstacleAABB);

			// 最小重なりのある軸を見つけて押し出す
			if (overlap.x < overlap.y && overlap.x < overlap.z) {
				// X軸方向の押し出し
				float centerX = (ghostAABB.min.x + ghostAABB.max.x) * 0.5f;
				float obstacleCenterX = (obstacleAABB.min.x + obstacleAABB.max.x) * 0.5f;

				if (centerX < obstacleCenterX) {
					worldTransform_.translation_.x -= overlap.x;
				}
				else {
					worldTransform_.translation_.x += overlap.x;
				}

				// 目的地も更新
				if (std::abs(randomDestination_.x - obstacleCenterX) < 10.0f) {
					SetNewRandomDestination();
				}
			}
			else if (overlap.z < overlap.x && overlap.z < overlap.y) {
				// Z軸方向の押し出し
				float centerZ = (ghostAABB.min.z + ghostAABB.max.z) * 0.5f;
				float obstacleCenterZ = (obstacleAABB.min.z + obstacleAABB.max.z) * 0.5f;

				if (centerZ < obstacleCenterZ) {
					worldTransform_.translation_.z -= overlap.z;
				}
				else {
					worldTransform_.translation_.z += overlap.z;
				}

				// 目的地も更新
				if (std::abs(randomDestination_.z - obstacleCenterZ) < 10.0f) {
					SetNewRandomDestination();
				}
			}
			else {
				// Y軸方向の押し出し（通常は重力で制御）
				float centerY = (ghostAABB.min.y + ghostAABB.max.y) * 0.5f;
				float obstacleCenterY = (obstacleAABB.min.y + obstacleAABB.max.y) * 0.5f;

				if (centerY < obstacleCenterY) {
					worldTransform_.translation_.y -= overlap.y;
					velocityY_ = 0.0f;  // 下から衝突した場合は上向きの速度をリセット
				}
				else {
					worldTransform_.translation_.y += overlap.y;
					velocityY_ = 0.0f;  // 上から衝突した場合（着地）
					onGround_ = true;
				}
			}
		}
	}
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

bool GhostEnemy::IsPlayerInChaseRadius() {
	if (!player_) return false;

	Vector3 playerWorldPosition = player_->GetWorldPosition();
	Vector3 enemyWorldPosition = GetWorldPosition();
	float distance = Length(playerWorldPosition - enemyWorldPosition);
	return distance <= chaseRadius_;
}

void GhostEnemy::SetFieldBoundaries(const Vector3& min, const Vector3& max) {
	fieldMin_ = min;
	fieldMax_ = max;
}
// GhostEnemy.cpp 内に追加する関数

void GhostEnemy::CheckAndResolveGhostCollisions() {
	if (!otherGhosts_ || isPlayer) return;

	for (auto* otherGhost : *otherGhosts_) {
		// 自分自身は無視
		if (otherGhost == this || otherGhost->isPlayer) continue;

		// お互いのAABBを取得
		AABB myAABB = GetAABB();
		AABB otherAABB = otherGhost->GetAABB();

		// 衝突判定
		if (IsCollisionAABB(myAABB, otherAABB)) {
			// 衝突した場合、押し出し方向と距離を計算
			Vector3 overlap = GetOverlapAmount(myAABB, otherAABB);
			Vector3 myCenterPos = {
				(myAABB.min.x + myAABB.max.x) * 0.5f,
				(myAABB.min.y + myAABB.max.y) * 0.5f,
				(myAABB.min.z + myAABB.max.z) * 0.5f
			};

			Vector3 otherCenterPos = {
				(otherAABB.min.x + otherAABB.max.x) * 0.5f,
				(otherAABB.min.y + otherAABB.max.y) * 0.5f,
				(otherAABB.min.z + otherAABB.max.z) * 0.5f
			};

			// 押し出し方向ベクトル（自分から相手への方向の逆）
			Vector3 pushDir = {
				myCenterPos.x - otherCenterPos.x,
				myCenterPos.y - otherCenterPos.y,
				myCenterPos.z - otherCenterPos.z
			};

			if (Length(pushDir) < 0.0001f) {
				// 完全に重なっている場合はランダムな方向に少しずらす
				pushDir.x = static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f;
				pushDir.z = static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f;
			}

			pushDir = Normalize(pushDir);

			// X軸とZ軸の重なり具合を比較し、より小さい方向に押し出す
			if (overlap.x < overlap.z) {
				// X軸方向の押し出し
				float pushAmount = overlap.x * 0.5f; // 半分ずつ押し出す
				worldTransform_.translation_.x += pushDir.x * pushAmount;
				// 相手側も押す（相互作用）
				otherGhost->worldTransform_.translation_.x -= pushDir.x * pushAmount;
			}
			else {
				// Z軸方向の押し出し
				float pushAmount = overlap.z * 0.5f; // 半分ずつ押し出す
				worldTransform_.translation_.z += pushDir.z * pushAmount;
				// 相手側も押す（相互作用）
				otherGhost->worldTransform_.translation_.z -= pushDir.z * pushAmount;
			}

			// 衝突したら少し速度を落とす（衝撃表現）
			velocity.x *= 0.9f;
			velocity.z *= 0.9f;

			// 衝突後の新しい目的地を設定する確率を上げる
			if (randomMoveTimer_ > 1.0f && std::rand() % 100 < 30) {
				SetNewRandomDestination();
			}
		}
	}
}