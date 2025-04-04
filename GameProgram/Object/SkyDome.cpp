#include "Skydome.h"

Skydome::Skydome(){}
Skydome::~Skydome() { delete model_; }

void Skydome::Initialize() {

	model_ = new Object3d();
	model_->Initialize();
	model_->SetModelFile("space");

	worldTransform_.Initialize();
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / -2.0f;
}

void Skydome::Update() {

	//worldTransform_.rotation_.y += 0.008f; // 回転速度は調整可能
	// 行列計算
	worldTransform_.UpdateMatrix();
}

void Skydome::Draw() {
	// 3Dモデル描画
	model_->Draw(worldTransform_);
}