#include "Bom.h"

Bom::Bom() {}
Bom::~Bom() { delete model_; }

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
		return;
	}
	position_.x += velocity_.x;
	position_.z += velocity_.z;	
	
	worldTransform_.translation_ = position_;

	worldTransform_.UpdateMatrix();

	deadTimer -= 1.0f / 60.0f;

	if (deadTimer < 0) {
		isDead = true;
	}
}

void Bom::Draw() {

	model_->Draw(worldTransform_);

}

AABB Bom::GetAABB() {
	float halfW = 0.9f, halfH = 0.9f, halfD = 0.9f; 
	AABB bomAABB;
	bomAABB.min = {worldTransform_.translation_.x - halfW, worldTransform_.translation_.y - halfH, worldTransform_.translation_.z - halfD};
	bomAABB.max = {worldTransform_.translation_.x + halfW, worldTransform_.translation_.y + halfH, worldTransform_.translation_.z + halfD};

	return bomAABB;
}

void Bom::OnCollision() { isDead = true; }
