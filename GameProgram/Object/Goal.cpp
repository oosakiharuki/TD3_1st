#include "Goal.h"
#include "MyMath.h"
#include <TextureManager.h>
#include "Input.h"
#include <numbers>
#include "ImGuiManager.h"
#include "Particle.h"
#include "Audio.h"

Goal::Goal() {}
Goal::~Goal() {
	delete sprite;
	delete model_;
	delete particleCelebration_;
}

void Goal::Init() {
	worldTransform_.Initialize();

	// モデルの読み込み
	model_ = new Object3d();
	model_->Initialize();
	model_->SetModelFile("goal");

	// 勝利画面用テクスチャの読み込み
	//TextureManager::GetInstance()->LoadTexture("resource/Sprite/winScene.png");

	sprite = new Sprite();
	sprite->Initialize("scene/winScene.png");
	sprite->SetPosition({0, 0});

	// 行列を更新
	worldTransform_.UpdateMatrix();

	// パーティクルの初期化
	particleCelebration_ = new Particle();
	// star.pngが存在しないため、代替のテクスチャを使用
	particleCelebration_->Initialize("resource/Sprite/circle.png"); // 円のパーティクル
	particleCelebration_->ChangeMode(BornParticle::Stop);
	particleCelebration_->ChangeType(ParticleType::Normal);

	// オーディオの初期化
	audio_ = Audio::GetInstance();
	clearSound_ = audio_->LoadWave("sound/clear.wav");
}


void Goal::Update() {
	const float deltaTime = 1.0f / 60.0f;

	// フローティングアニメーション
	floatingTime_ += deltaTime;
	float floatOffset = sin(floatingTime_ * 2.0f) * 0.5f;
	worldTransform_.translation_.y = baseY_ + floatOffset;

	// パーティクルの更新
	if (particleCelebration_) {
		particleCelebration_->Update();
	}

	if (isClear) {
		// クリア時のパーティクルエフェクト
		celebrationTimer_ += deltaTime;
		particleSpawnTimer_ += deltaTime;

		// 定期的にパーティクルを生成
		if (particleSpawnTimer_ > 0.1f) {
			particleCelebration_->SetTranslate(worldTransform_.translation_);
			particleCelebration_->SetParticleCount(20);
			particleCelebration_->SetFrequency(0.01f);
			particleCelebration_->SetScale({1.5f, 1.5f, 1.5f});
			particleCelebration_->ChangeType(ParticleType::Plane);
			particleCelebration_->ChangeMode(BornParticle::MomentMode);
			particleSpawnTimer_ = 0.0f;
		}

		if (Input::GetInstance()->TriggerKey(DIK_R)) {
			isClear = false;
			celebrationTimer_ = 0.0f;
			particleSpawnTimer_ = 0.0f;
		}
#ifdef _DEBUG
		ImGui::Begin("Restart");
		ImGui::Text("keyBorad 'R' Restart");
		ImGui::End();
#endif // _DEBUG
	}
	sprite->Update();
	worldTransform_.UpdateMatrix();
}

void Goal::Draw() { model_->Draw(worldTransform_); }

void Goal::DrawP() {
	// パーティクルの描画
	if (particleCelebration_) {
		particleCelebration_->Draw();
	}
}

void Goal::Text() {
	// isClearがtrueでスプライトが正常に初期化されている場合のみ描画
	if (isClear && sprite != nullptr) {
		//sprite->Draw();
	}
}

void Goal::OnCollision() {
	if (!isClear) {
		isClear = true;
		// クリア音を再生
		if (audio_) {
			audio_->SoundPlayWave(clearSound_, 1.0f);
		}
	}
}

AABB Goal::GetAABB() {
	float halfW = 2.0f, halfH = 2.0f, halfD = 2.0f;
	AABB aabb;
	aabb.min = {worldTransform_.translation_.x - halfW, worldTransform_.translation_.y - halfH, worldTransform_.translation_.z - halfD};
	aabb.max = {worldTransform_.translation_.x + halfW, worldTransform_.translation_.y + halfH, worldTransform_.translation_.z + halfD};
	return aabb;
}
