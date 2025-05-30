#include "CannonEnemy.h"
#include "Bom.h"
#include "Player.h"
#include "ImGuiManager.h"
#include <algorithm>
#include <iostream>

using namespace MyMath;

CannonEnemy::CannonEnemy() {}

CannonEnemy::~CannonEnemy() {
	delete model_;
	for (Bom* bom : bullets_) {
		delete bom;
	}
	delete particleMove_;
}

void CannonEnemy::Init() {
	worldTransform_.Initialize();

	model_ = new Object3d();
	model_->Initialize();
	model_->SetModelFile("cannon");
	particleMove_ = new Particle();
	particleMove_->Initialize("resource/Sprite/circle.png");
	particleMove_->ChangeMode(BornParticle::Stop);

	worldTransform_.translation_ = position;

	// オーディオシングルトン取得
	audio_ = Audio::GetInstance();

	// 爆発音の複数バッファを初期化（同時再生用）
	for (int i = 0; i < MAX_BOM_SOUNDS; i++) {
		bomSounds_[i] = audio_->LoadWave("sound/bom.wav");
	}
	currentBomSoundIndex_ = 0;
}

void CannonEnemy::SetObstacleList(const std::vector<AABB>& obstacles) { obstacleList_.insert(obstacleList_.end(), obstacles.begin(), obstacles.end()); }

void CannonEnemy::AddObstacle(const AABB& obstacle) { obstacleList_.push_back(obstacle); }

void CannonEnemy::Update() {

	// 入力による移動
	const float deltaTime = 1.0f / 60.0f;

	// スタン状態の処理
	if (isStan) {
		timerS += deltaTime;
		if (timerS > stanTime) {
			isStan = false;
			timerS = 0.0f;
		}
	}

	// 弾とステージの衝突チェック
	for (auto& obstacleAABB : obstacleList_) {
		for (Bom* bullet : bullets_) {
			AABB bulletAABB = bullet->GetAABB();
			if (IsCollisionAABB(bulletAABB, obstacleAABB)) {
				bullet->OnCollision();
			}
		}
	}


	if (!isPlayer) {
		// 重力処理
		float gravity = 0.01f;
		velocityY_ -= gravity;
		position.y += velocityY_;

		// 大砲のAABB更新
		float halfW = 1.0f, halfH = 1.0f, halfD = 1.0f;
		AABB enemyAABB;
		enemyAABB.min = { position.x - halfW, position.y - halfH, position.z - halfD };
		enemyAABB.max = { position.x + halfW, position.y + halfH, position.z + halfD };

		// 衝突解決
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
	
		// 大砲と壊せるブロックの衝突チェック
		for (Block* block : blocks_) {
			// ブロックがアクティブな場合のみ判定
			if (block->IsActive()) {
				AABB blockAABB = block->GetAABB();
				if (IsCollisionAABB(enemyAABB, blockAABB)) {
					ResolveAABBCollision(enemyAABB, blockAABB, velocityY_, onGround_);
				}
			}
		}

		// 衝突解決後の位置更新
		position.x = (enemyAABB.min.x + enemyAABB.max.x) * 0.5f;
		position.y = (enemyAABB.min.y + enemyAABB.max.y) * 0.5f;
		position.z = (enemyAABB.min.z + enemyAABB.max.z) * 0.5f;

		// 敵の攻撃ロジック
		if (!isStan && player_) {
			// プレイヤーのAABBとの距離を計算
			AABB playerAABB = player_->GetAABB();

			// プレイヤーとの最短距離を計算
			Vector3 closestPoint{
				std::clamp(position.x, playerAABB.min.x, playerAABB.max.x),
				std::clamp(position.y, playerAABB.min.y, playerAABB.max.y),
				std::clamp(position.z, playerAABB.min.z, playerAABB.max.z)
			};

			// プレイヤーとの距離を計算
			float distance = Length(closestPoint - position);

			// プレイヤーが攻撃範囲内に入ったら発射
			if (distance <= attackRadius) {
				// 発射メソッドを呼び出し
				Fire();
			}
			else {
				//発射モーションリセット
				worldTransform_.scale_ = { 1,1,1 };
				fireTimer = fireInterval;
				animertion = 0.0f;
			}
		}

		// プレイヤーの方向を向く処理
		if (player_) {
			Vector3 playerPosition = player_->GetWorldPosition();
			Vector3 direction = playerPosition - position;

			// atan2を使ってY軸の回転角度を計算
			worldTransform_.rotation_.y = atan2(direction.x, direction.z);
		}

		// 落下判定用
		const float fallThreshold = -60.0f;

		// バネとプレイヤーと大きさが若干違うので
		// ステージブロックの先に行くと落ちる
		// なので落ちたらスタート地点に戻るようにした
		if (position.y < fallThreshold) {
			position = RespownPosition;
		}


		worldTransform_.translation_ = position;
	}

#ifdef _DEBUG
	ImGui::Begin("Cannon Enemy");
	ImGui::DragFloat3("Position", &position.x);
	ImGui::DragFloat("Attack Radius", &attackRadius, 1.0f, 0.0f, 100.0f);
	ImGui::DragFloat("Fire Interval", &fireInterval, 0.1f, 1.0f, 10.0f);
	ImGui::Text("Fire Timer: %.1f", fireTimer);
	ImGui::End();
#endif

	// 弾の更新
	for (Bom* bom : bullets_) {
		bom->Update();
	}

	// 使用済み弾の削除
	bullets_.remove_if([](Bom* bom) {
		if (bom->IsDaed()) {
			delete bom;
			return true;
		}
		return false;
		});
	particleMove_->Update();
	worldTransform_.UpdateMatrix();
}

void CannonEnemy::Draw() {
	model_->Draw(worldTransform_);

	for (Bom* bom : bullets_) {
		bom->Draw();
	}
}

void CannonEnemy::DrawP() {
	particleMove_->Draw();

	// 弾のパーティクルも描画
	for (Bom* bom : bullets_) {
		bom->DrawP();
	}
}

void CannonEnemy::Fire() {
	const float deltaTimer = 1.0f / 60.0f;
	fireTimer -= deltaTimer;
	animertion += deltaTimer;

	Vector3 Scale = { 1.0f + animertion / 3.0f, 1.0f + animertion / 3.0f, 1.0f + animertion / 3.0f };

	// 発射間隔が経過したら発射
	if (fireTimer <= 0.0f) {

		particleMove_->SetParticleCount(20);
		particleMove_->SetFrequency(0.01f);
		particleMove_->SetTranslate(worldTransform_.translation_);

		particleMove_->ChangeType(ParticleType::Plane);
		particleMove_->ChangeMode(BornParticle::MomentMode);

		const float kSpeed = 0.3f; // 弾の速度（少し遅く）

		// プレイヤーの位置を取得
		Vector3 playerPosition = player_->GetWorldPosition();
		Vector3 enemyPosition = worldTransform_.translation_;

		// プレイヤーへの方向ベクトルを計算（ワールド座標系）
		Vector3 direction = playerPosition - enemyPosition;
		Vector3 normalizedDirection = Normalize(direction); // 正規化

		// 速度ベクトルをプレイヤーの方向に設定
		Vector3 velocity = normalizedDirection * kSpeed;

		// 新しい弾を生成
		Bom* newBullet = new Bom();
		newBullet->Init(enemyPosition, velocity);

		bullets_.push_back(newBullet);

		// 発射音を再生（複数バッファを順番に使用）（音量を小さく調整）
		audio_->SoundPlayWave(bomSounds_[currentBomSoundIndex_], 0.4f, false);


		// 次のサウンドバッファに進む
		currentBomSoundIndex_ = (currentBomSoundIndex_ + 1) % MAX_BOM_SOUNDS;

		// 次の発射までのクールダウンを設定（3秒に延長）
		fireTimer = fireInterval;
		animertion = 0.0f;
	}

	//発射モーション
	worldTransform_.scale_ = Scale;

}

void CannonEnemy::PlayerFire() {
	const float kSpeed = 0.5f;

	Vector3 velocity(0, 0, kSpeed);

	velocity = TransformNormal(velocity, worldTransform_.matWorld_);

	Vector3 playerPosition = player_->GetWorldPosition();
	playerPosition += TransformNormal({ 0,0,2 }, worldTransform_.matWorld_);

	Bom* newBullet = new Bom();
	newBullet->Init(playerPosition, velocity);

	bullets_.push_back(newBullet);

	// 発射音を再生
	audio_->StopWave(bomSounds_[currentBomSoundIndex_]);
	audio_->SoundPlayWave(bomSounds_[currentBomSoundIndex_], 0.4f, false);


	// 次のサウンドバッファに進む
	currentBomSoundIndex_ = (currentBomSoundIndex_ + 1) % MAX_BOM_SOUNDS;
}

// AABBを取得するメソッドを定義
AABB CannonEnemy::GetAABB() {
	float halfW = 1.0f, halfH = 1.0f, halfD = 1.0f;
	AABB enemyAABB;
	enemyAABB.min = { worldTransform_.translation_.x - halfW, worldTransform_.translation_.y - halfH, worldTransform_.translation_.z - halfD };
	enemyAABB.max = { worldTransform_.translation_.x + halfW, worldTransform_.translation_.y + halfH, worldTransform_.translation_.z + halfD };
	return enemyAABB;
}

void CannonEnemy::ContralPlayer() {
	isPlayer = true;

	// のりうつった時の弾の削除
	bullets_.remove_if([](Bom* bom) {
		delete bom;
		return true;
		});

	//発射モーションリセット
	worldTransform_.scale_ = { 1,1,1 };
	fireTimer = fireInterval;
	animertion = 0.0f;

	worldTransform_.translation_ = { 0, -2, 0 };
	worldTransform_.rotation_ = { 0, 0, 0 };
	if (player_) {
		player_->SetState(Player::State::Bomb);
	}
}

void CannonEnemy::ReMove(const Vector3& position_) {
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