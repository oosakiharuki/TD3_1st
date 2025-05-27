#include "Block.h"
#include "MyMath.h"
#include <cmath>
#include <random>
#include "Audio.h"

using namespace MyMath;

Block::Block() {}

Block::~Block() { 
	if (model_) {
		delete model_;
		model_ = nullptr;
	}
	if (particle) {
		delete particle;
		particle = nullptr;
	}
}

void Block::Init() {

	// worldTransformを確実に初期化
	worldTransform.Initialize();

	// モデルの読み込み
	model_ = new Object3d();
	model_->Initialize();
	model_->SetModelFile("cube");
	// 行列の更新
	worldTransform.UpdateMatrix();

	// 元の位置を保存
	originalPosition_ = worldTransform.translation_;

	//ブロックに攻撃した時のパーテイクル
	try {
		particle = new Particle();
		if (particle) {
			particle->Initialize("resource/Sprite/break_block.png");
			particle->ChangeMode(BornParticle::Stop);
			particle->ChangeType(ParticleType::Normal);
		}
	} catch (...) {
		OutputDebugStringA("Block::Init - Failed to create particle\n");
		if (particle) {
			delete particle;
			particle = nullptr;
		}
	}

	// オーディオ初期化
	audio_ = Audio::GetInstance();
	hitSound_ = audio_->LoadWave("sound/hit.wav");
	breakSound_ = audio_->LoadWave("sound/break.wav");
}

void Block::Update() {
	// パーティクルは常に更新（破壊エフェクトのため）
	if (particle) {
		particle->Update();
		particle->SetScale({ 0.8f,0.8f ,0.8f });
	}
	
	// 非アクティブなブロックは以降の処理をスキップ
	if (!isActive_) {
		return;
	}

	const float deltaTime = 1.0f / 60.0f;

	// ダメージエフェクトの更新
	if (isDamaged_) {
		damageTimer_ -= deltaTime;
		scaleTimer_ -= deltaTime;
		shakeTimer_ -= deltaTime;

		// 揺れエフェクト
		if (shakeTimer_ > 0.0f) {
			static std::random_device rd;
			static std::mt19937 gen(rd());
			static std::uniform_real_distribution<float> dis(-1.0f, 1.0f);

			Vector3 shakeOffset = {
				dis(gen) * SHAKE_INTENSITY,
				dis(gen) * SHAKE_INTENSITY,
				dis(gen) * SHAKE_INTENSITY
			};
			worldTransform.translation_ = originalPosition_ + shakeOffset;
		}
		else {
			// 揺れ終了時は元の位置に戻す
			worldTransform.translation_ = originalPosition_;
		}

		// スケールエフェクト（毎フレーム元のサイズから計算）
		if (scaleTimer_ > 0.0f) {
			// 最初の半分で大きくなり、後半で戻る
			float progress = 1.0f - (scaleTimer_ / SCALE_EFFECT_TIME);
			float scaleMultiplier;
			if (progress < 0.5f) {
				// 拡大フェーズ
				scaleMultiplier = 1.0f + (progress * 2.0f * (SCALE_INTENSITY - 1.0f));
			}
			else {
				// 縮小フェーズ
				scaleMultiplier = SCALE_INTENSITY - ((progress - 0.5f) * 2.0f * (SCALE_INTENSITY - 1.0f));
			}
			// 元のサイズから計算（累積しない）
			worldTransform.scale_ = size_ * scaleMultiplier;
		}
		else {
			// スケールエフェクト終了時は元のサイズに戻す
			worldTransform.scale_ = size_;
		}

		// エフェクト終了判定
		if (damageTimer_ <= 0.0f) {
			isDamaged_ = false;
			worldTransform.scale_ = size_; // スケールを元に戻す
			worldTransform.translation_ = originalPosition_; // 位置を元に戻す
		}
	}
	else {
		// エフェクト中でない時は通常状態を維持
		worldTransform.scale_ = size_;
		worldTransform.translation_ = originalPosition_;
	}

	worldTransform.UpdateMatrix();
}

void Block::Draw() {
	// 条件式の修正: !isActive_ ではなく isActive_ のときに描画
	if (isActive_) {
		model_->Draw(worldTransform);
	}
	// 非アクティブなら描画しない (ここでreturnを入れるなら上のifブロックの外に出す)
}

void Block::DrawP() {
	// 非アクティブなブロックのパーティクルも描画する（破壊エフェクトのため）
	if (particle) {
		particle->Draw();
	}
}

AABB Block::GetAABB() const {
	AABB aabb;
	aabb.min = worldTransform.translation_ - size_;
	aabb.max = worldTransform.translation_ + size_;
	return aabb;
}

void Block::SetPosition(const Vector3& pos) {
	worldTransform.translation_ = pos;
	originalPosition_ = pos; // 元の位置も更新
	worldTransform.UpdateMatrix();
}

void Block::SetSize(const Vector3& size) {
	worldTransform.scale_ = size;
	size_ = size;
	worldTransform.UpdateMatrix();
}

void Block::OnCollision() {
	// すでに破壊されている場合は処理しない
	if (!isActive_ || hp <= 0) {
		return;
	}
	
	// パーティクルの有効性をチェック
	if (!particle) {
		OutputDebugStringA("Block::OnCollision - particle is nullptr\n");
		return;
	}
	
	hp--;

	// ダメージエフェクトを開始
	isDamaged_ = true;
	damageTimer_ = DAMAGE_EFFECT_TIME;
	scaleTimer_ = SCALE_EFFECT_TIME;
	shakeTimer_ = SHAKE_EFFECT_TIME;

	// HPに応じてエフェクトを変更（控えめに調整）
	if (hp <= 1) {
		// HPが低い時は少し大きめのエフェクト
		SCALE_INTENSITY = 1.15f;
		SHAKE_INTENSITY = 0.08f;
	}
	else if (hp <= 2) {
		// HPが中程度の時は中程度のエフェクト
		SCALE_INTENSITY = 1.12f;
		SHAKE_INTENSITY = 0.06f;
	}
	else {
		// HPが満タンの時は通常のエフェクト
		SCALE_INTENSITY = 1.1f;
		SHAKE_INTENSITY = 0.05f;
	}

	// パーティクルエフェクト（HPに応じて段階的に変更）
	if (hp <= 0) {
		// 破壊時は最も派手なパーティクル
		particle->SetParticleCount(50); // 大幅に増加
		particle->SetFrequency(0.001f); // より密度を高く
		particle->ChangeType(ParticleType::Plane);
		particle->SetScale({2.0f, 2.0f, 2.0f}); // 大きめのパーティクル
		// 破壊音を再生
		audio_->SoundPlayWave(breakSound_, 0.8f);
		// 最後にisActive_をfalseにする
		isActive_ = false;
	}
	else if (hp == 1) {
		// HP1の時は中程度のパーティクル
		particle->SetParticleCount(20); // 増加
		particle->SetFrequency(0.005f);
		particle->ChangeType(ParticleType::Normal);
		particle->SetScale({1.5f, 1.5f, 1.5f});
	}
	else if (hp == 2) {
		// HP2の時は小さなパーティクル
		particle->SetParticleCount(10); // 増加
		particle->SetFrequency(0.01f);
		particle->ChangeType(ParticleType::Normal);
		particle->SetScale({1.2f, 1.2f, 1.2f});
	}
	else {
		// HP3の時は最小のパーティクル
		particle->SetParticleCount(5); // 増加
		particle->SetFrequency(0.015f);
		particle->ChangeType(ParticleType::Normal);
		particle->SetScale({1.0f, 1.0f, 1.0f});
		// ヒット音を再生
		audio_->SoundPlayWave(hitSound_, 0.6f);
	}

	// パーティクル位置の設定（セットされていない場合はブロックの位置を使用）
	Vector3 effectPosition = particlePosition;
	if (effectPosition.x == 0.0f && effectPosition.y == 0.0f && effectPosition.z == 0.0f) {
		// particlePositionがセットされていない場合はブロックの位置を使用
		effectPosition = worldTransform.translation_;
	}
	
	// パーティクル位置のデバッグ出力
	char debugMsg[256];
	sprintf_s(debugMsg, "Block::OnCollision - Setting particle position to (%.2f, %.2f, %.2f)\n", 
		effectPosition.x, effectPosition.y, effectPosition.z);
	OutputDebugStringA(debugMsg);
	
	particle->SetTranslate(effectPosition);
	particle->ChangeMode(BornParticle::MomentMode);
}
