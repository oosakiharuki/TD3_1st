#include "Bom.h"
#include "ParticleManager.h"

Bom::Bom() {
	// Audioインスタンスを取得
	audio_ = Audio::GetInstance();
	soundData_ = nullptr;

	// パーティクル初期化
	particleExplosion_ = new Particle();
	particleExplosion_->Initialize("resource/Sprite/circle.png");
	particleExplosion_->ChangeMode(BornParticle::Stop);
	particleExplosion_->ChangeType(ParticleType::Plane);
}

Bom::~Bom() { 
	delete model_; 
	delete particleExplosion_;

	// 音を停止する
	if (soundData_ != nullptr) {
		audio_->StopWave(*soundData_);
	}
}

void Bom::Init(Vector3 position, Vector3 velocity) {
	worldTransform_.Initialize();
	// "cube" モデルを読み込み
	model_ = new Object3d();
	model_->Initialize();
	model_->SetModelFile("EnemyBullet");
	position_ = position;
	worldTransform_.translation_ = position_;

	velocity_ = velocity;
}

void Bom::Update() { 
	//プレイヤーが真上の時出さなくする
	if (velocity_.x == 0.0f && velocity_.z == 0.0f) {
		isDead = true;
		// 音を止める
		if (soundData_ != nullptr) {
			audio_->StopWave(*soundData_);
		}
		// 即座に非表示にする
		isVisible = false;
		return;
	}
	position_.x += velocity_.x;
	position_.z += velocity_.z;	
	
	worldTransform_.translation_ = position_;

	worldTransform_.UpdateMatrix();

	// パーティクル更新
	particleExplosion_->Update();

	deadTimer -= 1.0f / 60.0f;

	if (deadTimer < 0) {
		isDead = true;
		// 音を止める
		if (soundData_ != nullptr) {
			audio_->StopWave(*soundData_);
		}
		// 即座に非表示にする
		isVisible = false;
	}
}

void Bom::Draw() {
	// 表示フラグがオンの場合のみ描画
	if (isVisible) {
		model_->Draw(worldTransform_);
	}
}

void Bom::DrawP() {
	particleExplosion_->Draw();
}

AABB Bom::GetAABB() {
	float halfW = 0.9f, halfH = 0.9f, halfD = 0.9f; 
	AABB bomAABB;
	bomAABB.min = {worldTransform_.translation_.x - halfW, worldTransform_.translation_.y - halfH, worldTransform_.translation_.z - halfD};
	bomAABB.max = {worldTransform_.translation_.x + halfW, worldTransform_.translation_.y + halfH, worldTransform_.translation_.z + halfD};

	return bomAABB;
}

void Bom::OnCollision() { 
	isDead = true; 
	// 音を止める
	if (soundData_ != nullptr) {
		audio_->StopWave(*soundData_);
	}
	// 即座に非表示にする
	isVisible = false;

	// 爆発パーティクルを発生
	particleExplosion_->SetParticleCount(30);
	particleExplosion_->SetFrequency(0.001f);
	particleExplosion_->SetTranslate(worldTransform_.translation_);
	particleExplosion_->SetScale({1.5f, 1.5f, 1.5f});
	particleExplosion_->ChangeMode(BornParticle::MomentMode);
}

void Bom::SetSoundData(SoundData* soundData) {
	soundData_ = soundData;
}