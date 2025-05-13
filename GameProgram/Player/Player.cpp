#include "Player.h"
#ifdef _DEBUG
#include "ImGuiManager.h"
#endif
#include "GhostEnemy.h"
#include "Door.h"

#include <algorithm>
#include <iostream>

#include"Audio.h"

#include "ParticleManager.h"

using namespace MyMath;

Player::Player() {}

Player::~Player() {
	delete particleMove_;
	delete particleTransfar_;
	delete headModel_;
	delete footModel_;
}

void Player::Init(Camera* camera) {
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	//頭用のワールド座標
	worldTransformH_.Initialize();
	headModel_ = new Object3d();
	headModel_->Initialize();
	headModel_->SetModelFile("playerHead");
	worldTransformH_.translation_ = position;

	//足用のワールド座標
	worldTransformF_.Initialize();
	footModel_ = new Object3d();
	footModel_->Initialize();
	footModel_->SetModelFile("playerFoot");
	worldTransformF_.translation_ = position;

	//アニメーション初期化
	InitializeFloatingGimmick();


	// 初期位置を保存
	initialPosition = position;

	audio_ = Audio::GetInstance();

	JumpSound_ = audio_->LoadWave("sound/jump.wav");
	SnapSound_ = audio_->LoadWave("sound/snap.wav");
	DamageSound_ = audio_->LoadWave("sound/fall.wav");//読み取れんかった
	FallSound_ = audio_->LoadWave("sound/fall.wav"); // 仮に落下音もジャンプ音と同じものを使用

	particleMove_ = new Particle();
	particleMove_->Initialize("resource/Sprite/circle.png");
	particleMove_->ChangeMode(BornParticle::Stop);

	particleTransfar_ = new Particle();
	particleTransfar_->Initialize("resource/Sprite/circle.png");
	particleTransfar_->ChangeMode(BornParticle::Stop);

	//切り替え時長押しにならないように
	state = Input::GetInstance()->GetState();
	preState = Input::GetInstance()->GetPreState();
}

void Player::SetObstacleList(const std::vector<AABB>& obstacles) { obstacleList_.insert(obstacleList_.end(), obstacles.begin(), obstacles.end()); }

void Player::AddObstacle(const AABB& obstacle) { obstacleList_.push_back(obstacle); }

void Player::Update() {
#pragma region 入力処理
	Input::GetInstance()->GetJoystickState(0, state);
	Input::GetInstance()->GetJoystickStatePrevious(0, preState);

	// キーボードとGamePad左スティックの入力を合算して移動処理する
	float keyboardSpeed = 0.55f;
	Vector3 inputVec = { 0.0f, 0.0f, 0.0f };

	// キーボード入力 (WASD)
	if (Input::GetInstance()->PushKey(DIK_W)) {
		inputVec.z += keyboardSpeed;
	}
	if (Input::GetInstance()->PushKey(DIK_S)) {
		inputVec.z -= keyboardSpeed;
	}
	if (Input::GetInstance()->PushKey(DIK_A)) {
		inputVec.x -= keyboardSpeed;
	}
	if (Input::GetInstance()->PushKey(DIK_D)) {
		inputVec.x += keyboardSpeed;
	}

	// GamePad左スティック入力
	const float deadZone = 0.2f;
	if (Input::GetInstance()->GetJoystickState(0, state)) {
		float gpX = static_cast<float>(state.Gamepad.sThumbLX) / 32768.0f;
		float gpZ = static_cast<float>(state.Gamepad.sThumbLY) / 32768.0f;
		if (fabs(gpX) < deadZone)
			gpX = 0.0f;
		if (fabs(gpZ) < deadZone)
			gpZ = 0.0f;
		inputVec.x += gpX;
		inputVec.z += gpZ;
	}

	// 入力処理とプレイヤーの目標移動速度を計算
	Vector3 targetVelocity = {0.0f, 0.0f, 0.0f};
	if (Length(inputVec) > 0) {
		Vector3 move = Normalize(inputVec) * speed;
		float yawRad = cameraYaw * (3.14159265f / 180.0f);

		// カメラ向きに合わせて回転
		Vector3 rotatedMove;
		rotatedMove.x = move.x * cosf(yawRad) + move.z * sinf(yawRad);
		rotatedMove.z = -move.x * sinf(yawRad) + move.z * cosf(yawRad);
		rotatedMove.y = 0.0f;

		// 目標速度を設定
		targetVelocity = rotatedMove;

		// 目標の回転角を計算
		float targetRotateY = std::atan2(rotatedMove.x, rotatedMove.z);
		
		// 回転もスムーズにする
		const float rotationSmoothFactor = 0.2f;
		RotateY = LeapShortAngle(RotateY, targetRotateY, rotationSmoothFactor);
	}

	// スムージング係数（当たりをゆるくしつつ、動きは速めに）
	const float moveSmoothFactor = 0.3f;
	
	// 現在の速度から目標速度へ徐々に変化
	velocity.x = velocity.x + (targetVelocity.x - velocity.x) * moveSmoothFactor;
	velocity.z = velocity.z + (targetVelocity.z - velocity.z) * moveSmoothFactor;

	// 速度が非常に小さい場合はゼロにする（浮動小数点の問題回避）
	const float stopThreshold = 0.001f;
	if (Length(velocity) < stopThreshold) {
		velocity.x = 0.0f;
		velocity.z = 0.0f;
	}

	// 計算された速度で移動
	position.x += velocity.x;
	position.z += velocity.z;

#pragma endregion

#pragma region 状態切替
	// 状態切替
	if (Input::GetInstance()->TriggerKey(DIK_1)) {
		currentState = State::Normal;
	}
	if (Input::GetInstance()->TriggerKey(DIK_2)) {
		currentState = State::Bomb;
	}
	if (Input::GetInstance()->TriggerKey(DIK_3)) {
		currentState = State::Ghost;
	}
#pragma endregion

#pragma region カメラ操作
	// コントローラとキーボード両方で回さないようにするフラグ
	bool isKeyBorad = false;

	// キーボードによるカメラ回転X
	if (Input::GetInstance()->PushKey(DIK_LEFT)) {
		cameraYaw -= 2.5f;
		isKeyBorad = true;
	}
	if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
		cameraYaw += 2.5f;
		isKeyBorad = true;
	}

	// キーボードによるカメラ回転Y
	if (Input::GetInstance()->PushKey(DIK_DOWN)) {
		cameraPitch += 1.5f;
		isKeyBorad = true;
	}
	if (Input::GetInstance()->PushKey(DIK_UP)) {
		cameraPitch -= 1.5f;
		isKeyBorad = true;
	}

	// GamePad右スティックによるカメラ回転処理
	float xCamera = 0.0f, zCamera = 0.0f;
	if (Input::GetInstance()->GetJoystickState(0, state) && !isKeyBorad) {

		// 右スティックの入力
		xCamera = static_cast<float>(state.Gamepad.sThumbRX) / 32768.0f; // -1.0f～1.0f
		zCamera = static_cast<float>(state.Gamepad.sThumbRY) / 32768.0f; // -1.0f～1.0f

		// デッドゾーン処理
		if (abs(xCamera) < deadZone) {
			xCamera = 0.0f;
			if (fabs(zCamera) < deadZone)
				zCamera = 0.0f;
		}

		// カメラ向き
		// Y軸
		cameraYaw += xCamera * 2.5f;
		// X軸
		cameraPitch += zCamera * 2.5f;
	}

	cameraPitch = std::clamp(cameraPitch, 0.0f, 80.0f);

	cameraController_.SetPitch(cameraPitch);
	cameraController_.SetYaw(cameraYaw);
	worldTransform_.rotation_.y = LeapShortAngle(worldTransform_.rotation_.y, RotateY, 0.2f);
	worldTransformH_.rotation_.y = LeapShortAngle(worldTransform_.rotation_.y, RotateY, 0.2f);
	worldTransformF_.rotation_.y = LeapShortAngle(worldTransform_.rotation_.y, RotateY, 0.2f);


	//if (velocityY_ == 0.0f) {
	//	onGround_ = true;
	//}
	//else {
	//	onGround_ = false;
	//}

#pragma endregion

#pragma region ジャンプ・移動処理
	// ジャンプ・移動時の各種処理
	if (!onGround_) {
		if ((state.Gamepad.wButtons & XINPUT_GAMEPAD_A) && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_A) && !EnemyContral && !isTransfar) {
			velocityY_ -= 1.2f;
			isTransfar = true;
		}
		else if (Input::GetInstance()->TriggerKey(DIK_SPACE) && !EnemyContral && !isTransfar) {
			velocityY_ -= 1.2f;
			isTransfar = true;
		}
	}
	else {
		isTransfar = false;
	}

	if ((state.Gamepad.wButtons & XINPUT_GAMEPAD_A) && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_A) && velocityY_ == 0.0f) {
		velocityY_ = 0.3f;
		onGround_ = false;

		// ジャンプ音を再生
		audio_->SoundPlayWave(JumpSound_, 0.7f);
	}
	else if (Input::GetInstance()->TriggerKey(DIK_SPACE) && velocityY_ == 0.0f) {
		velocityY_ = 0.3f;
		onGround_ = false;

		// ジャンプ音を再生
		audio_->SoundPlayWave(JumpSound_, 0.7f);
	}

	if ((state.Gamepad.wButtons & XINPUT_GAMEPAD_B) && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_B) && velocityY_ == 0.0f && EnemyContral) {
		velocityY_ = 0.0f;
		EnemyContral = false;
		onEnemy = true;
	}
	else if (Input::GetInstance()->TriggerKey(DIK_K) && velocityY_ == 0.0f && EnemyContral) {
		velocityY_ = 0.0f;
		EnemyContral = false;
		onEnemy = true;
	}
#pragma endregion

#pragma region 物理演算と衝突処理
	// 重力と垂直移動
	float gravity = 0.01f;
	velocityY_ -= gravity;
	position.y += velocityY_;

	// プレイヤーのAABB更新
	float halfW = 1.0f, halfH = 1.0f, halfD = 1.0f;
	playerAABB.min = { position.x - halfW, position.y - halfH, position.z - halfD };
	playerAABB.max = { position.x + halfW, position.y + halfH, position.z + halfD };

	// 障害物との衝突解決
	const int maxIterations = 10;
	int iterations = 0;
	bool collisionOccurred = false;
	do {
		collisionOccurred = false;
		for (auto& obstacleAABB : obstacleList_) {
			if (IsCollisionAABB(playerAABB, obstacleAABB)) {
				ResolveAABBCollision(playerAABB, obstacleAABB, velocityY_, onGround_);
				collisionOccurred = true;
			}
		}
		iterations++;
	} while (collisionOccurred && iterations < maxIterations);
#pragma endregion

#pragma region 敵との衝突処理
	// キャノン敵との衝突処理 - 全てのキャノン敵をチェック
	for (CannonEnemy* cannon : cannonEnemies_) {
		AABB cannonAABB = cannon->GetAABB();
		if (IsCollisionAABB(playerAABB, cannonAABB)) {
			ResolveAABBCollision(playerAABB, cannonAABB, velocityY_, onGround_);
			if (isTransfar && (playerAABB.min.y >= cannonAABB.max.y) && !EnemyContral) {
				cannon->ContralPlayer();
				EnemyContral = true;
				collisionEnemy = true;

				// 乗り移り音を再生
				audio_->SoundPlayWave(SnapSound_, 0.6f);
			}
		}

		if (EnemyContral && cannon->GetPlayerCtrl()) {
			cannon->SetParent(&worldTransform_);
			if ((state.Gamepad.wButtons & XINPUT_GAMEPAD_X) && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_X)) {
				cannon->PlayerFire();
			}
			else if (Input::GetInstance()->TriggerKey(DIK_J)) {
				cannon->PlayerFire();
			}
		}
		else {
			cannon->ReMove(worldTransform_.translation_);
		}
	}

	CheckCollision();

	OnCollisions();


	for (SpringEnemy* springEnemy : springEnemies_) {
		AABB springAABB = springEnemy->GetAABB();

		if (IsCollisionAABB(playerAABB, springAABB)) {
			ResolveAABBCollision(playerAABB, springAABB, velocityY_, onGround_);
			if (isTransfar && (playerAABB.min.y >= springAABB.max.y) && !EnemyContral) {
				springEnemy->ContralPlayer();
				EnemyContral = true;
				collisionEnemy = true;

				// 乗り移り音を再生
				audio_->SoundPlayWave(SnapSound_, 0.6f);
			}
		}

		if (EnemyContral && springEnemy->GetPlayerCtrl()) {
			springEnemy->SetParent(&worldTransform_);
		}
		else {
			springEnemy->ReMove(worldTransform_.translation_);
		}
	}

	// ドアとの衝突処理
	for (Door* doorAABB : doors_) {
		if (IsCollisionAABB(playerAABB, doorAABB->GetAABB()) && !doorAABB->IsDoorOpened()) {
			ResolveAABBCollision(playerAABB, doorAABB->GetAABB(), velocityY_, onGround_);
		}
	}

	// Tileとの衝突判定
	for (auto& tileAABB : moveTileAABBs) {
		if (IsCollisionAABB(playerAABB, tileAABB)) {
			ResolveAABBCollision(playerAABB, tileAABB, velocityY_, onGround_);
		}
	}
	moveTileAABBs.clear(); // 次のフレームのために配列をクリア

	// 敵との衝突処理
	for (auto it = ghostEnemies_.begin(); it != ghostEnemies_.end();) {
		enemyAABB = (*it)->GetAABB();
		if (IsCollisionAABB(playerAABB, enemyAABB)) {

			// 真上に乗れて、横は透ける
			Vector3 overlap = GetOverlapAmount(playerAABB, enemyAABB);
			if (overlap.y < overlap.x && overlap.y < overlap.z) {
				float playerCenterY = (playerAABB.min.y + playerAABB.max.y) * 0.5f;
				float obstacleCenterY = (enemyAABB.min.y + enemyAABB.max.y) * 0.5f;
				float push = (playerCenterY < obstacleCenterY) ? -overlap.y : overlap.y;
				playerAABB.min.y += push;
				playerAABB.max.y += push;
				// 上向きの押し戻しなら着地判定を立てる
				if (push > 0.0f) {
					velocityY_ = 0.0f;
					onGround_ = true;
				}
			}
			else { //　横に当たったらダメージ
				isDamage = true;
			}

			if (isTransfar && (playerAABB.min.y >= enemyAABB.max.y) && !EnemyContral) {
				(*it)->ContralPlayer();
				EnemyContral = true;
				collisionEnemy = true;

				// 乗り移り音を再生
				audio_->SoundPlayWave(SnapSound_, 0.6f);
			}
		}

		if (EnemyContral && (*it)->GetPlayerCtrl()) {
			(*it)->SetParent(&worldTransform_);
		}
		else {
			(*it)->ReMove(worldTransform_.translation_);
		}
		++it;
	}
#pragma endregion

#pragma region 位置更新
	// AABBの中心を基に位置を更新
	position.x = (playerAABB.min.x + playerAABB.max.x) * 0.5f;
	position.y = (playerAABB.min.y + playerAABB.max.y) * 0.5f;
	position.z = (playerAABB.min.z + playerAABB.max.z) * 0.5f;
	if (onEnemy) {
		position.y += 2.0f;
		onEnemy = false;
	}

	//particleMove_->ChangeMode(BornParticle::Stop);	
	//particleTransfar_->ChangeMode(BornParticle::Stop);

	if (worldTransform_.translation_.x != position.x || velocityY_ != 0.0f || worldTransform_.translation_.z != position.z) {
		particleMove_->SetParticleCount(2);
		particleMove_->SetFrequency(0.25f);
		
		if (EnemyContral) {
			//操っている敵のほうにパーテイクルを出す
			particleMove_->SetTranslate({ worldTransform_.translation_.x,worldTransform_.translation_.y - 2.0f,worldTransform_.translation_.z });
		}
		else {
			particleMove_->SetTranslate(worldTransform_.translation_);
		}

		particleMove_->ChangeType(ParticleType::Normal);
		particleMove_->ChangeMode(BornParticle::TimerMode);
		isMoving = true;
	}
	else {
		particleMove_->ChangeMode(BornParticle::Stop);
		isMoving = false;
	}
	
	
	if (collisionEnemy) {
		particleTransfar_->SetParticleCount(10);
		particleTransfar_->SetTranslate({ worldTransform_.translation_.x,worldTransform_.translation_.y - 1.0f,worldTransform_.translation_.z });
		particleTransfar_->ChangeType(ParticleType::Plane);
		particleTransfar_->ChangeMode(BornParticle::MomentMode);
	}


	if (EnemyContral && collisionEnemy) {
		position.y -= 2.0f;	
		collisionEnemy = false;
	}
	worldTransform_.translation_ = position;
	
	worldTransformH_.translation_ = position;
	worldTransformF_.translation_ = position;


	if (EnemyContral) {
		worldTransform_.translation_.y += 2.0f;
		worldTransformH_.translation_.y += 0.8f;
	}	
	
	particleMove_->Update();
	particleTransfar_->Update();

#pragma endregion

#pragma region 追加チェック処理
	// ばね敵との衝突チェック
	CheckCollisionWithSprings();

	//　攻撃されたら
	CheckDamage();

	//ゴールの旗に当たったか
	CheckCollisionWithGoal();

	// 落下判定チェック
	CheckFallOut();
#pragma endregion

#pragma region デバッグ表示
#ifdef _DEBUG
	ImGui::Begin("player");
	ImGui::DragFloat3("translate", &worldTransform_.translation_.x);
	ImGui::DragFloat3("aabbMax", &playerAABB.max.x);
	ImGui::DragFloat3("aabbMin", &playerAABB.min.x);

	ImGui::DragFloat3("translate", &worldTransform_.rotation_.x);

	ImGui::Text("HP: %d", hp);
	ImGui::Text("Position Y: %.2f (Fall at: %.2f)", position.y, fallThreshold);
	ImGui::End();
#endif
#pragma endregion

	//速すぎてものが貫通しないようにする
	velocityY_ = std::clamp(velocityY_, -20.0f, 20.0f);

	UpdateFloatingGimmick();
	
	worldTransform_.UpdateMatrix();
	worldTransformH_.UpdateMatrix();
	worldTransformF_.UpdateMatrix();

	cameraController_.Update(camera_, position);
}

// 落下チェック関数
void Player::CheckFallOut() {
	// Y座標が閾値を下回った場合
	if (position.y < fallThreshold) {
		// 初期位置にリセット
		ResetToInitialPosition();

		// ダメージを与える（通常の2倍）
		TakeDamage(fallDamage);

		// 落下音を再生
		audio_->SoundPlayWave(FallSound_, 0.7f);
	}
}

// 初期位置にリセットする関数
void Player::ResetToInitialPosition() {
	position = initialPosition;
	velocityY_ = 0.0f;
	worldTransform_.translation_ = position;
	onGround_ = true;
}

void Player::CheckCollision() {
	//if (!block_->IsActive() && !ghostBlock_->IsActive()) {
	//	return;
	//}

	for (auto* block_ : blocks_) {
		AABB blockAABB = block_->GetAABB();
		switch (currentState) {
		case State::Bomb:
			// 全てのキャノン敵の弾でブロック破壊チェック
			for (CannonEnemy* cannon : cannonEnemies_) {
				for (Bom* bom : cannon->GetBom()) {
					AABB bomAABB = bom->GetAABB();
					if (block_->IsActive() && IsCollisionAABB(bomAABB, blockAABB) && cannon->GetPlayerCtrl()) {
						block_->SetActive(false);
					}
				}
			}
			break;
		}
		if (block_->IsActive() && IsCollisionAABB(playerAABB, blockAABB)) {
			ResolveAABBCollision(playerAABB, blockAABB, velocityY_, onGround_);
		}
	}

	for (auto* ghostBlock_ : ghostBlocks_) {
		AABB ghostBlockAABB = ghostBlock_->GetAABB();


		switch (currentState) {
		case State::Normal:
			if (ghostBlock_->IsActive() && IsCollisionAABB(playerAABB, ghostBlockAABB)) {
				ResolveAABBCollision(playerAABB, ghostBlockAABB, velocityY_, onGround_);
			}
			break;

		case State::Bomb:
			if (ghostBlock_->IsActive() && IsCollisionAABB(playerAABB, ghostBlockAABB)) {
				ResolveAABBCollision(playerAABB, ghostBlockAABB, velocityY_, onGround_);
			}

			break;

		case State::Ghost:
			for (auto* it : ghostEnemies_) {
				if (ghostBlock_->IsActive() && IsCollisionAABB(playerAABB, ghostBlockAABB) && it->GetPlayerCtrl()) {
					if (ghostBlock_->GetColor() == it->GetColor()) {
						velocity.x = 0;
					}
					else {
						ResolveAABBCollision(playerAABB, ghostBlockAABB, velocityY_, onGround_);
					}

				}
			}
			break;
		}
	}

}

void Player::DrawUI() {

#ifdef _DEBUG

	ImGui::Begin("Player State");

	const char* stateNames[] = { "Normal", "Bomb", "Ghost" };
	ImGui::Text("Current State: %s", stateNames[static_cast<int>(currentState)]);
	ImGui::Text("HP: %d", hp);
	ImGui::Text("Position Y: %.2f", position.y);

	ImGui::End();

#endif // _DEBUG
}

void Player::OnCollisions() {
	// 全てのキャノン敵の弾を確認
	for (CannonEnemy* cannon : cannonEnemies_) {
		for (Bom* bom : cannon->GetBom()) {
			AABB bomAABB = bom->GetAABB();

			if (IsCollisionAABB(bomAABB, playerAABB) && !cannon->GetPlayerCtrl()) {
				isDamage = true;
				bom->OnCollision();
			}
		}
	}
}

void Player::Draw() {
	// 点滅中で非表示の場合は描画しない
	if (isFlashing && !isVisible) {
		return;
	}

	// 通常の描画（テクスチャハンドルは使わない）

	//頭のモデル描画
	headModel_->Draw(worldTransformH_);
	//足のモデル描画
	if (!EnemyContral) {
		footModel_->Draw(worldTransformF_);
	}
}

void Player::DrawP() {
	particleMove_->Draw();
	particleTransfar_->Draw();
}

void Player::SetGhostEnemies(const std::vector<GhostEnemy*>& enemies) { ghostEnemies_ = enemies; }

void Player::SetSpringEnemies(const std::vector<SpringEnemy*>& springEnemies) { springEnemies_ = springEnemies; }

void Player::SetCannon(CannonEnemy* cannon) {
	// 後方互換性のため - 既存をクリアしてこれを追加
	cannonEnemies_.clear();
	if (cannon) {
		cannonEnemies_.push_back(cannon);
	}
}

void Player::SetCannonEnemies(const std::vector<CannonEnemy*>& cannons) {
	cannonEnemies_ = cannons;
}

void Player::TakeDamage() {
	isDamage = true;
}

// 指定されたダメージを受ける関数
void Player::TakeDamage(int damageAmount) {
	if (!isFlashing) { // 無敵時間中でなければダメージを受ける
		hp -= damageAmount;
		if (hp < 0) hp = 0; // HPが0未満にならないようにする

		isDamage = false;
		coolTime = flashDuration; // 1.5秒間の点滅時間
		isFlashing = true;
		flashTimer = 0.0f;
		isVisible = true;

		Audio::GetInstance()->SoundPlayWave(DamageSound_, 0.7f);// ダメージを食らったときのSE
	}
}

void Player::CheckCollisionWithSprings() {
	for (auto* springEnemy : springEnemies_) {
		AABB springAABB = springEnemy->GetAABB();

		if (IsCollisionAABB(playerAABB, springAABB)) {
			// Resolve collision
			ResolveAABBCollision(playerAABB, springAABB, velocityY_, onGround_);

			// If player is landing on top of the spring
			if (velocityY_ <= 0 && playerAABB.min.y >= springAABB.max.y - 0.2f) {
				// Apply the jump boost (much higher than normal jump)
				velocityY_ = 0.6f * springEnemy->GetJumpBoost();
				onGround_ = false;
				springEnemy->Compress(); // Trigger visual feedback
			}
		}
	}
}


void Player::SetState(State newState) { currentState = newState; }

void Player::ClearObstacleList() {
	obstacleList_.clear(); // 当たり判定用の障害物リストをクリア
}

void Player::SetPosition(const Vector3& newPosition) {
	position = newPosition;
	worldTransform_.translation_ = position;
	// 初期位置も更新
	initialPosition = newPosition;
}


void Player::CheckCollisionWithGoal() {
	// ゴールが設定されていない場合は何もしない
	if (!goal_) {
		return;
	}

	AABB goalAABB = goal_->GetAABB();

	if (IsCollisionAABB(playerAABB, goalAABB)) {
		goal_->OnCollision();
	}
}

void Player::CheckDamage() {
	const float deltaTime = 1.0f / 60.0f;

	// 点滅処理の更新
	if (isFlashing) {
		flashTimer += deltaTime;

		// 0.3秒ごとに表示/非表示を切り替え
		if (flashTimer >= flashInterval) {
			isVisible = !isVisible; // 表示状態を反転
			flashTimer = 0.0f;      // タイマーリセット
		}

		// 点滅の継続時間（1.5秒）が終了したら
		coolTime -= deltaTime;
		if (coolTime <= 0.0f) {
			isFlashing = false;
			isVisible = true; // 必ず表示状態に戻す
			coolTime = 0.0f;
		}
	}

	// ダメージを受けたときの処理
	if (isDamage && !isFlashing) {
		TakeDamage(20); // 通常のダメージ量を20とする
		isDamage = false;
	}
	else {
		isDamage = false;
	}
}


/// <summary>
/// アニメーションのプログラム
/// </summary>

void Player::InitializeFloatingGimmick() {
	floatingParameter_ = 0.0f;
}

void Player::UpdateFloatingGimmick() {
	const float fream = 60.0f;

	const float step = 2.0f * 3.14f / fream;


	floatingParameter_ += step;

	floatingParameter_ = std::fmod(floatingParameter_, 2.0f * 3.14f);

	//ふよふよす動き
	const float wide = 0.1f;
	worldTransformH_.translation_.y += std::sin(floatingParameter_) * wide;
	worldTransformF_.translation_.y += std::sin(floatingParameter_) * wide;

	//動いている時
	if (isMoving) {
		const float footRotate = 0.5f;
		worldTransformF_.rotation_.y += std::sin(floatingParameter_) * footRotate;
	}

	const float footScale = 0.005f;
	worldTransformF_.scale_.x += std::sin(floatingParameter_) * footScale;
	worldTransformF_.scale_.z += std::sin(floatingParameter_) * footScale;
}