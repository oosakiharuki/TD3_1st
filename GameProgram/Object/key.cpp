#include "Key.h"

#ifdef _DEBUG
#include "ImGuiManager.h"
#endif

Key::Key() {}

Key::~Key() { 
	delete model_; 
	delete particle;
}

void Key::Init() {

	worldTransform_.Initialize();
	
	// "cube" モデルを読み込み

	model_ = new Object3d();
	model_->Initialize();
	model_->SetModelFile("key");

	// 位置を設定
	worldTransform_.translation_ = position_;

	// サイズを小さめに設定（見た目調整用）
	worldTransform_.scale_ = {0.5f, 0.5f, 0.5f};

	// 行列を更新
	worldTransform_.UpdateMatrix();

	// 鍵取得音の読み込み
	audio_ = Audio::GetInstance();
	keyGetSound_ = audio_->LoadWave("sound/key_get.wav");

	particle = new Particle();
	particle->Initialize("resource/Sprite/get_key.png");
}

void Key::Update() {

	particle->Update();

	// 既に取得されていたら処理しない
	if (isObtained_) {
		particle->ChangeMode(BornParticle::Stop);
		return;
	}

	// プレイヤーとの衝突判定
	if (player_) {
		AABB playerAABB = player_->GetAABB();
		AABB keyAABB = GetAABB();

		if (IsCollisionAABB(playerAABB, keyAABB)) {
			// 衝突したら鍵を取得
			isObtained_ = true;

			// 鍵取得音を再生
			KeyGetAudio_ = -1;
		}
	}

	if (KeyGetAudio_ < 0) {
		audio_->SoundPlayWave(keyGetSound_, 0.5);
		KeyGetAudio_++;
	}

	// 回転アニメーション
	if (!isObtained_) {
		rotationY_ += 0.02f;
		worldTransform_.rotation_.y = rotationY_;
		
		particle->SetParticleCount(1);
		particle->SetFrequency(0.50f);
		particle->SetTranslate({ worldTransform_.translation_.x,worldTransform_.translation_.y,worldTransform_.translation_.z });
	}

	// 行列を更新
	worldTransform_.UpdateMatrix();

	// ImGuiデバッグ表示
#ifdef _DEBUG
	ImGui::Begin("Key Status");
	ImGui::Text("Key ID: %d", keyID_);
	ImGui::Checkbox("KeyFlg", &isObtained_);
	ImGui::End();
#endif
}

void Key::Draw() {
	// 取得されていない場合のみ描画
	if (!isObtained_) {
		model_->Draw(worldTransform_);
	}
}
void Key::DrawP() {
	particle->Draw();
}

// AABBを取得するメソッド
AABB Key::GetAABB() const {
	float halfW = 0.5f * worldTransform_.scale_.x;
	float halfH = 0.5f * worldTransform_.scale_.y;
	float halfD = 0.5f * worldTransform_.scale_.z;

	AABB keyAABB;
	keyAABB.min = {worldTransform_.translation_.x - halfW, worldTransform_.translation_.y - halfH, worldTransform_.translation_.z - halfD};
	keyAABB.max = {worldTransform_.translation_.x + halfW, worldTransform_.translation_.y + halfH, worldTransform_.translation_.z + halfD};

	return keyAABB;
}