#include "Goal.h"
#include "MyMath.h"
#include <TextureManager.h>
#include "Input.h"
#include <numbers>
#include "ImGuiManager.h"

Goal::Goal() {}
Goal::~Goal() {
	delete sprite;
	delete model_;
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
}


void Goal::Update() {
	if (isClear) {

		if (Input::GetInstance()->TriggerKey(DIK_R)) {
			isClear = false;
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

void Goal::Text() {
	// isClearがtrueでスプライトが正常に初期化されている場合のみ描画
	if (isClear && sprite != nullptr) {
		sprite->Draw();
	}
}

void Goal::OnCollision() { isClear = true; }

AABB Goal::GetAABB() {
	float halfW = 2.0f, halfH = 2.0f, halfD = 2.0f;
	AABB aabb;
	aabb.min = {worldTransform_.translation_.x - halfW, worldTransform_.translation_.y - halfH, worldTransform_.translation_.z - halfD};
	aabb.max = {worldTransform_.translation_.x + halfW, worldTransform_.translation_.y + halfH, worldTransform_.translation_.z + halfD};
	return aabb;
}
